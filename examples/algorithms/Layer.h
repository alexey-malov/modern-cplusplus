#pragma once

#include <memory>
#include <vector>

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

	void AddSublayer(const LayerPtr& subLayer)
	{
		InsertSublayerAtIndex(subLayer, m_sublayers.size());
	}

	LayerPtr GetSuperlayer()const
	{
		return m_superlayer.lock();
	}

	void InsertSublayerAtIndex(const LayerPtr & sublayer, size_t index)
	{
		sublayer->m_superlayer = shared_from_this();
		m_sublayers.push_back(sublayer);
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