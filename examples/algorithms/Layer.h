#pragma once

#include <memory>
#include <vector>
#include <algorithm>

//class Layer;
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
		for (auto parent = GetSuperlayer(); parent; parent = parent->GetSuperlayer())
		{
			if (parent == layer)
			{
				throw std::invalid_argument("Can't insert any of own superlayers");
			}
		}
		if (layer->GetSuperlayer().get() != this)
		{
			m_sublayers.insert(m_sublayers.begin() + insertPos, layer);
			layer->m_superlayer = shared_from_this();
		}
		else // Перемещаем слой
		{
			if (insertPos > m_sublayers.size())
			{
				throw std::out_of_range("Index is out of range");
			}
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
				//m_sublayers.insert(m_sublayers.begin() + insertPos, layer);
				//m_sublayers.erase(m_sublayers.begin() + layerPos + 1);
				//*
				for (; layerPos != insertPos; --layerPos)
				{
					m_sublayers[layerPos] = m_sublayers[layerPos - 1];
				}
				m_sublayers[layerPos] = layer;
				//*/
			}
			else // (insertPos >= layerPos)
			{
				//m_sublayers.insert(m_sublayers.begin() + insertPos, layer);
				//m_sublayers.erase(m_sublayers.begin() + layerPos);
				for (; layerPos < insertPos - 1; ++layerPos)
				{
					m_sublayers[layerPos] = m_sublayers[layerPos + 1];
				}
				m_sublayers[layerPos] = layer;
			}
		}
	}

	void RemoveFromSuperlayer()
	{
		auto superlayer = GetSuperlayer();
		if (superlayer)
		{
			auto self = shared_from_this();
			auto & siblings = superlayer->m_sublayers;
			for (auto it = siblings.begin(); it != siblings.end(); ++it)
			{
				if (*it == self)
				{
					siblings.erase(it);
					break;
				}
			}
			m_superlayer.reset();
		}
	}
};