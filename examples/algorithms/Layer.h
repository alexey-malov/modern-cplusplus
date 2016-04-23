#pragma once

#include <memory>
#include <vector>
#include <algorithm>

typedef std::shared_ptr<class Layer> LayerPtr;

class Layer : public std::enable_shared_from_this<Layer>
{
	std::weak_ptr<Layer> m_superlayer;
	std::vector<LayerPtr> m_sublayers;
public:
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
		if (layer.get() == this)
		{
			throw std::invalid_argument("Can't insert itself as a sublayer");
		}
		if (insertPos > m_sublayers.size())
		{
			throw std::out_of_range("Index is out of range");
		}

		for (auto parent = GetSuperlayer(); parent; parent = parent->GetSuperlayer())
		{
			if (parent == layer)
			{
				throw std::invalid_argument("Can't insert any of own superlayers");
			}
		}

		auto oldOwner = layer->GetSuperlayer();
		if (oldOwner.get() != this)
		{
			m_sublayers.insert(m_sublayers.begin() + insertPos, layer);

			if (oldOwner)
			{
				auto & oldSiblings = oldOwner->m_sublayers;
				for (auto it = oldSiblings.begin(); it != oldSiblings.end(); ++it)
				{
					if (*it == layer)
					{
						oldSiblings.erase(it);
						break;
					}
				}
			}
			layer->m_superlayer = shared_from_this();
		}
		else // Это наш собственный слой
		{
			size_t layerPos;
			for (layerPos = 0; layerPos < m_sublayers.size(); ++layerPos)
			{
				if (m_sublayers[layerPos] == layer)
				{
					break;
				}
			}
			assert(layerPos < m_sublayers.size());

			if (insertPos < layerPos)
			{
				for (; layerPos > insertPos; --layerPos)
				{
					m_sublayers[layerPos] = m_sublayers[layerPos - 1];
				}
			}
			else
			{
				for (; layerPos + 1 < insertPos; ++layerPos)
				{
					m_sublayers[layerPos] = m_sublayers[layerPos + 1];
				}
			}
			m_sublayers[layerPos] = layer;
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