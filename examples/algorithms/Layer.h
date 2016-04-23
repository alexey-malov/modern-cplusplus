#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <iterator>
#include <boost/range/algorithm/find.hpp>

typedef std::shared_ptr<class Layer> LayerPtr;

class Layer : public std::enable_shared_from_this<Layer>
{
	std::weak_ptr<Layer> m_superlayer;
	std::vector<LayerPtr> m_sublayers;
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

		auto self = shared_from_this();
		if (std::find(ParentIterator(self), ParentIterator(), layer))
		{
			throw std::invalid_argument("Can't insert self of an own superlayer");
		}

		auto oldOwner = layer->GetSuperlayer();
		if (oldOwner.get() != this)
		{
			m_sublayers.insert(m_sublayers.begin() + insertPos, layer);
			if (oldOwner)
			{				
				auto & oldSiblings = oldOwner->m_sublayers;
				oldSiblings.erase(boost::find(oldSiblings, layer));
			}
			layer->m_superlayer = self;
		}
		else // Это наш собственный слой
		{
			auto first = boost::find(m_sublayers, layer);
			auto last = first + 1;
			auto pos = m_sublayers.begin() + insertPos;

			if (pos < first)
			{
				rotate(pos, first, last);
			}
			else if (last < pos)
			{
				rotate(first, last, pos);
			}
		}
	}
	
	void RemoveFromSuperlayer()
	{
		auto superlayer = GetSuperlayer();
		if (superlayer)
		{
			auto & siblings = superlayer->m_sublayers;

			for (auto it = siblings.begin(); it != siblings.end(); ++it)
			{
				if (it->get() == this)
				{
					siblings.erase(it);
					break;
				}
			}
			m_superlayer.reset();
		}
	}
};