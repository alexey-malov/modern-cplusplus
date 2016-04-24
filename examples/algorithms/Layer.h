#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>
#include <boost/range/algorithm/find.hpp>

typedef std::shared_ptr<class Layer> LayerPtr;

template <typename It>
std::pair<It, It> slide(const It & first, const It & last, const It & pos)
{
	if (pos < first) return { pos, rotate(pos, first, last) };
	if (last < pos) return { rotate(first, last, pos), pos };
	return { first, last };
}

class Layer : public std::enable_shared_from_this<Layer>
{
public:

	class ParentIterator : public std::iterator<std::input_iterator_tag, LayerPtr>
	{
		LayerPtr p;
	public:
		ParentIterator(const LayerPtr & p = nullptr)
			:p(p)
		{}

		bool operator==(const ParentIterator & rhs)const
		{
			return p == rhs.p;
		}
		bool operator!=(const ParentIterator & rhs)const
		{
			return p != rhs.p;
		}

		operator bool()const
		{
			return p != nullptr;
		}
		ParentIterator& operator++()
		{
			assert(p);
			p = p->GetSuperlayer();
			return *this;
		}
		ParentIterator operator++(int)
		{
			auto copy(*this);
			++(*this);
			return copy;
		}
		const LayerPtr& operator*()const
		{
			assert(p);
			return p;
		}
		const LayerPtr* operator->()const
		{
			assert(p);
			return &p;
		}
	};

	void Draw() const 
	{
		// будет переопределен в подклассах
	}

	void AddSublayer(const LayerPtr& layer)
	{
		InsertSublayerAtIndex(layer, m_sublayers.size());
	}

	LayerPtr GetSuperlayer()const
	{
		return m_superlayer.lock();
	}

	LayerPtr GetSublayer(size_t index)const
	{
		return m_sublayers.at(index);
	}

	size_t GetSublayerCount()const
	{
		return m_sublayers.size();
	}

	void InsertSublayerAtIndex(const LayerPtr & layer, size_t insertPos)
	{
		if (!layer)
		{
			throw std::invalid_argument("Invalid layer");
		}
		if (insertPos > m_sublayers.size())
		{
			throw std::out_of_range("Index is out of range");
		}

		const auto self = shared_from_this();
		if (std::find(ParentIterator(self), ParentIterator(), layer))
		{
			throw std::invalid_argument("Can't insert self of an own superlayer");
		}

		if (self != layer->GetSuperlayer())
		{
			AdoptSublayer(layer, insertPos);
		}
		else // Это наш собственный слой
		{
			auto oldPos = boost::find(m_sublayers, layer);
			slide(oldPos, oldPos + 1, m_sublayers.begin() + insertPos);
		}
	}

	void RemoveFromSuperlayer()
	{
		if (auto superlayer = GetSuperlayer())
		{
			auto & siblings = superlayer->m_sublayers;
			siblings.erase(boost::find(siblings, shared_from_this()));
			m_superlayer.reset();
		}
	}

private:
	void AdoptSublayer(const LayerPtr & layer, size_t insertPos)
	{
		m_sublayers.insert(m_sublayers.begin() + insertPos, layer);
		if (auto oldOwner = layer->GetSuperlayer())
		{
			auto & oldSiblings = oldOwner->m_sublayers;
			oldSiblings.erase(boost::find(oldSiblings, layer));
		}
		layer->m_superlayer = shared_from_this();
	}

	std::weak_ptr<Layer> m_superlayer;
	std::vector<LayerPtr> m_sublayers;

};