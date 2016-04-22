#pragma once

#include <memory>
#include <vector>
#include <algorithm>

class Layer;
typedef std::shared_ptr<Layer> LayerPtr;

class IDrawable
{
public:
	virtual void Draw() = 0;
	virtual ~IDrawable() = default;
};

class Layer : public IDrawable, public std::enable_shared_from_this<Layer>
{
	std::weak_ptr<Layer> m_superlayer;
	std::vector<LayerPtr> m_sublayers;
public:
	void Draw() override
	{
	}

	void AddSublayer(const LayerPtr& layer)
	{
		InsertSublayerAtIndex(layer, m_sublayers.size());
	}

	LayerPtr GetSuperlayer()const
	{
		return m_superlayer.lock();
	}

	void InsertSublayerAtIndex(const LayerPtr & layer, size_t index)
	{
		if (layer->GetSuperlayer().get() != this)
		{
			m_sublayers.insert(m_sublayers.begin() + index, layer);
			layer->m_superlayer = shared_from_this();
		}
		else // Just move the layer
		{
			if (index > m_sublayers.size())
			{
				throw std::out_of_range("Index is out of range");
			}
			size_t curPos = find(m_sublayers.begin(), m_sublayers.end(), layer) - m_sublayers.begin();
			if (index < curPos)
			{
				m_sublayers.insert(m_sublayers.begin() + index, layer);
				m_sublayers.erase(m_sublayers.begin() + curPos + 1);
			}
			else
			{
				m_sublayers.insert(m_sublayers.begin() + index, layer);
				m_sublayers.erase(m_sublayers.begin() + curPos);
			}
		}
	}

	LayerPtr GetSublayer(size_t index)const
	{
		return m_sublayers.at(index);
	}

	size_t GetSublayerCount()const
	{
		return m_sublayers.size();
	}

	void RemoveFromSuperlayer()
	{

	}
};