#pragma once

#include <cstddef>  // size_t, ptrdiff_t
#include <cstdint>  // uint32_t
#include <memory>  // pointer_traits
#include <utility>  // move, swap, forward
#include <iterator>  // forward_iterator_tag
#include <type_traits>  // enable_if, is_pointer

#include "RE/Memory.h"  // TES_HEAP_REDEFINE_NEW


namespace RE
{
	// forward list
	template <class T>
	class BSSimpleList
	{
	public:
		using value_type = T;
		using size_type = std::uint32_t;
		using difference_type = std::ptrdiff_t;
		using reference = value_type &;
		using const_reference = const value_type &;
		using pointer = value_type *;
		using const_pointer = std::pointer_traits<pointer>::rebind<const value_type>;

	protected:
		struct Node
		{
			constexpr Node() :
				item{},
				next{ 0 }
			{}


			template <class... Args>
			explicit Node(Args&&... a_args) :
				item(std::forward<Args>(a_args)),
				next(0)
			{}


			TES_HEAP_REDEFINE_NEW();


			value_type	item;
			Node*		next;
		};

	public:
		template <class U>
		struct iterator_traits
		{
			using difference_type = std::ptrdiff_t;
			using value_type = U;
			using pointer = U *;
			using reference = U &;
			using iterator_category = std::forward_iterator_tag;
		};


		template <class U>
		class iterator_base : public iterator_traits<U>
		{
			friend class BSSimpleList<T>;

		public:
			iterator_base() :
				_cur{ 0 }
			{}


			iterator_base(Node* a_node)
			{
				_cur = a_node;
			}


			iterator_base(const iterator_base& a_rhs) :
				_cur(a_rhs._cur)
			{}


			iterator_base(iterator_base&& a_rhs) :
				_cur(std::move(a_rhs._cur))
			{
				a_rhs._cur = 0;
			}


			static void swap(iterator_base& a_lhs, iterator_base& a_rhs)
			{
				std::swap(a_lhs._cur, a_rhs._cur);
			}


			iterator_base& operator=(const iterator_base& a_rhs)
			{
				iterator_base tmp(a_rhs);
				swap(*this, tmp);
			}


			iterator_base& operator=(iterator_base&& a_rhs)
			{
				_cur = std::move(a_rhs._cur);
				a_rhs._cur = 0;
			}


			[[nodiscard]] reference operator*() const
			{
				return _cur->item;
			}


			[[nodiscard]] pointer operator->() const
			{
				return std::pointer_traits<pointer>::pointer_to(operator*());
			}


			[[nodiscard]] bool operator==(const iterator_base& a_rhs) const
			{
				return _cur == a_rhs._cur;
			}


			[[nodiscard]] bool operator!=(const iterator_base& a_rhs) const
			{
				return !operator==(a_rhs);
			}


			// prefix
			iterator_base& operator++()
			{
				_cur = _cur->next;
				return *this;
			}


			// postfix
			iterator_base operator++(int)
			{
				iterator_base tmp{ *this };
				operator++();
				return tmp;
			}

		protected:
			Node* _cur;
		};


		using iterator = iterator_base<T>;
		using const_iterator = iterator_base<const T>;


		constexpr BSSimpleList() :
			_listHead{}
		{}


		~BSSimpleList()
		{
			clear();
		}


		TES_HEAP_REDEFINE_NEW();


		reference front()
		{
			return *begin();
		}


		const_reference front() const
		{
			return *cbegin();
		}


		iterator begin() noexcept
		{
			return empty() ? end() : iterator{ &_listHead };
		}


		const_iterator begin() const noexcept
		{
			return empty() ? end() : const_iterator{ const_cast<Node*>(&_listHead) };
		}


		const_iterator cbegin() const noexcept
		{
			return empty() ? cend() : const_iterator{ const_cast<Node*>(&_listHead) };
		}


		iterator end() noexcept
		{
			return { 0 };
		}


		const_iterator end() const noexcept
		{
			return { 0 };
		}


		const_iterator cend() const noexcept
		{
			return { 0 };
		}


		[[nodiscard]] bool empty() const noexcept
		{
			return !_listHead.next && !_listHead.item;
		}


		void clear() noexcept
		{
			while (!empty()) {
				pop_front();
			}
		}


		iterator insert_after(const_iterator a_pos, const T& a_value)
		{
			Node* node = new Node();
			node->item = a_value;
			node->next = a_pos._cur->next;
			a_pos._cur->next = node;
			return iterator{ node };
		}


		iterator insert_after(const_iterator a_pos, T&& a_value)
		{
			Node* node = new Node();
			node->item = std::move(a_value);
			node->next = a_pos._cur->next;
			a_pos._cur->next = node;
			return iterator{ node };
		}


		iterator insert_after(const_iterator a_pos, size_type a_count, const T& a_value)
		{
			Node* node = 0;
			while (a_count--) {
				node = new Node();
				node->item = a_value;
				node->next = a_pos._cur->next;
				a_pos._cur->next = node;
				++a_pos;
			}
			return iterator{ a_pos._cur };
		}


		template <class InputIt>
		iterator insert_after(const_iterator a_pos, InputIt a_first, InputIt a_last)
		{
			Node* node = 0;
			while (a_first != a_last) {
				node = new Node();
				node->item = *a_first;
				node->next = a_pos._cur->next;
				a_pos._cur->next = node;
				++a_pos;
				++a_first;
			}
			return iterator{ a_pos._cur };
		}


		iterator insert_after(const_iterator a_pos, std::initializer_list<T> a_ilist)
		{
			Node* node = 0;
			for (auto& elem : a_ilist) {
				node = new Node();
				node->item = elem;
				node->next = a_pos._cur->next;
				a_pos._cur->next = node;
				++a_pos;
			}
			return iterator{ a_pos._cur };
		}


		template <class... Args>
		iterator emplace_after(const_iterator a_pos, Args&&... a_args)
		{
			Node* node = new Node(a_args);
			node->next = a_pos._cur->next;
			a_pos._cur->next = node;
			return iterator{ node };
		}


		iterator erase_after(const_iterator a_pos)
		{
			const_iterator before = a_pos++;
			before._cur->next = a_pos._cur->next;
			delete a_pos._cur;
			return before._cur->next == 0 ? end() : iterator{ ++before._cur };
		}


		iterator erase_after(const_iterator a_first, const_iterator a_last)
		{
			const_iterator pos{ a_first };
			while (++pos != a_last) {
				delete pos._cur;
			}
			a_first._cur->next = a_last._cur;
			return iterator{ a_last._cur };
		}


		void push_front(const T& a_value)
		{
			Node* node = new Node();
			node->item = a_value;
			node->next = begin()._cur;
			_listHead = *node;
		}


		void push_front(T&& a_value)
		{
			Node* node = new Node();
			node->item = std::move(a_value);
			node->next = begin()._cur;
			_listHead = *node;
		}


		template <class... Args>
		void emplace_front(Args&&... a_args)
		{
			Node* node = new Node(a_args);
			node->next = begin()._cur;
			_listHead = *node;
		}


		template <class... Args>
		reference emplace_front(Args&&... a_args)
		{
			Node* node = new Node(a_args);
			node->next = begin()._cur;
			_listHead = *node;
			return *begin();
		}


		void pop_front()
		{
			iterator pos = begin();
			delete pos++._cur;
			_listHead = *pos._cur;
		}


		void resize(size_type a_count)
		{
			if (begin() == end()) {
				if (a_count == 0) {
					return;
				} else {
					Node* node = new Node();
					_listHead = *node;
				}
			}

			iterator pos = begin();
			size_type elems = 1;
			while (pos._cur->next != 0 && elems != a_count) {
				++elems;
				++pos;
			}

			if (pos._cur->next != 0) {
				// need to shrink
				pos++._cur->next = 0;
				while (pos != end()) {
					delete pos++._cur;
				}
			} else if (elems == a_count) {
				// already required size
				return;
			} else {
				// need to expand
				Node* node = 0;
				while (elems < a_count) {
					node = new Node();
					pos._cur->next = node;
					++pos;
					++elems;
				}
			}
		}


		void resize(size_type a_count, const value_type& a_value)
		{
			if (begin() == end()) {
				if (a_count == 0) {
					return;
				} else {
					Node* node = new Node();
					_listHead = *node;
				}
			}

			iterator pos = begin();
			size_type elems = 1;
			while (pos._cur->next != 0 && elems != a_count) {
				++elems;
				++pos;
			}

			if (pos._cur->next != 0) {
				// need to shrink
				pos++._cur->next = 0;
				while (pos != end()) {
					delete pos++._cur;
				}
			} else if (elems == a_count) {
				// already required size
				return;
			} else {
				// need to expand
				Node* node = 0;
				while (elems < a_count) {
					node = new Node(a_value);
					pos._cur->next = node;
					++pos;
					++elems;
				}
			}
		}


		void swap(BSSimpleList& a_other) noexcept
		{
			Node* tmp = &_listHead;
			_listHead = a_other._listHead;
			a_other._listHead = *tmp;
		}

	protected:
		Node _listHead;	// 00
	};
}
