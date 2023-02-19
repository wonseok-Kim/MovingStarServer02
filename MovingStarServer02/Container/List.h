#pragma once

#pragma once

#include <stdlib.h>

template<typename T>
class List
{
public:
	struct Node
	{
		T data;
		Node* prev;
		Node* next;
	};

	class iterator;
	class const_iterator;

public:
	List();
	~List();

	iterator begin() { return iterator(m_Head.next); }
	iterator end() { return iterator(&m_Tail); }
	const_iterator begin() const { return const_iterator(m_Head.next); }
	const_iterator end() const { return const_iterator(&m_Tail); }

	void push_front(const T& data);
	void push_back(const T& data);
	void pop_front();
	void pop_back();

	T& front() { return (m_Head.next->data); }
	T& back() { return (m_Tail.prev->data); }
	const T& front() const { return (m_Head.next->data); }
	const T& back() const { return (m_Tail.prev->data); }

	int size() const { return m_Size; }
	bool empty() const { return m_Size == 0; }

	iterator erase(iterator iter) { return erase(iter.m_Node); }
	const_iterator erase(const_iterator iter) { return erase(iter.m_Node); }
	void remove(const T& data);
	void clear();

private:
	Node* erase(Node* toDelete);

private:
	Node m_Head;
	Node m_Tail;
	int m_Size;
};

template<typename T>
inline List<T>::List()
{
	m_Head.prev = nullptr;
	m_Head.next = &m_Tail;

	m_Tail.prev = &m_Head;
    m_Tail.next = nullptr;

	m_Size = 0;
}

template<typename T>
inline List<T>::~List()
{
	clear();
}

template<typename T>
inline void List<T>::push_front(const T& data)
{
	Node* node = new Node;
	node->data = data;

	node->next = m_Head.next;
	node->prev = &m_Head;

	m_Head.next->prev = node;
	m_Head.next = node;

	m_Size++;
}

template<typename T>
inline void List<T>::push_back(const T& data)
{
	Node* node = new Node;
	node->data = data;

	node->next = &m_Tail;
	node->prev = m_Tail.prev;

	m_Tail.prev->next = node;
	m_Tail.prev = node;

	m_Size++;
}

template<typename T>
inline void List<T>::pop_front()
{
	if (m_Size == 0)
		return;

	Node* del = m_Head.next;

	m_Head.next = del->next;
	del->next->prev = &m_Head;

	delete del;
	--m_Size;
}

template<typename T>
inline void List<T>::pop_back()
{
	if (m_Size == 0)
		return;

	Node* del = m_Tail.prev;

	m_Tail.prev = del->prev;
	del->prev->next = &m_Tail;

	delete del;
	--m_Size;
}

template<typename T>
inline void List<T>::clear()
{
	List<T>::iterator iter;
	for (iter = begin(); iter != end(); )
	{
		iter = erase(iter);
	}
}

template<typename T>
inline void List<T>::remove(const T& data)
{
	List<T>::iterator iter;
	for (iter = begin(); iter != end(); )
	{
		if (data == *iter)
		{
			iter = erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

template<typename T>
typename inline List<T>::Node* List<T>::erase(Node* toDelete)
{
	Node* del = toDelete;
	Node* ret = toDelete->next;

	del->prev->next = del->next;
	del->next->prev = del->prev;

	delete del;
	m_Size--;

	return ret;
}

template<typename T>
class  List<T>::iterator
{
public:
	iterator(Node* node = nullptr)
		: m_Node{ node }
	{ }

	friend iterator List<T>::erase(iterator iter);
	friend class const_iterator;

	iterator operator++(int)
	{
		iterator iter(m_Node);
		m_Node = m_Node->next;
		return iter;
	}

	iterator operator++()
	{
		m_Node = m_Node->next;
		return *this;
	}

	iterator operator--(int)
	{
        iterator iter(m_Node);
		m_Node = m_Node->prev;
		return iter;
	}

	iterator operator--()
	{
        m_Node = m_Node->prev;
		return *this;
	}

	T& operator*()
	{
		return m_Node->data;
	}

	T& operator->()
	{
		return m_Node->data;
	}

	bool operator==(const iterator& other) const
	{
		return m_Node == other.m_Node;
	}

	bool operator!=(const iterator& other) const
	{
		return m_Node != other.m_Node;
	}

private:
	Node* m_Node;
};

template<typename T>
class List<T>::const_iterator
{
public:
	const_iterator(const Node* node = nullptr)
		: m_Node{ const_cast<Node*>(node) }
	{ }

	friend const_iterator List<T>::erase(const_iterator iter);

	const_iterator(iterator iter)
		:m_Node{ iter.m_Node }
	{}

	const_iterator operator++(int) const
	{
		const_iterator iter(m_Node);
		m_Node = m_Node->next;
		return iter;
	}

	const_iterator operator++() 
	{
		m_Node = m_Node->next;
		return *this;
	}

	const_iterator operator--(int) 
	{
        const_iterator iter(m_Node);
        m_Node = m_Node->prev;
        return iter;
	}

	const_iterator operator--() 
	{
        m_Node = m_Node->prev;
		return *this;
	}

	const T& operator*() const
	{
		return m_Node->data;
	}

	const T* operator->() const
	{
		return &m_Node->data;
	}

	bool operator==(const const_iterator& other) const
	{
		return m_Node == other.m_Node;
	}

	bool operator!=(const const_iterator& other) const
	{
		return m_Node != other.m_Node;
	}

private:
	Node* m_Node;
};
