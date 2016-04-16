#include <memory>
#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>
#include <sstream>

constexpr auto M_PI = 3.14159265358979323846;

using namespace std;


class Shape
{
public:
	virtual double GetArea() const = 0;
	virtual string ToString() const = 0;
	virtual ~Shape() = default;
};

class Circle : public Shape
{
public:
	Circle(double radius) : m_radius(radius)
	{
	}
	string ToString() const override
	{
		return "Circle, radius: " + to_string(m_radius)
			+ ", area: " + to_string(GetArea());
	}
	double GetArea() const override
	{
		return M_PI * m_radius * m_radius;;
	}
private:
	double m_radius;
};

class Rectangle : public Shape
{
public:
	Rectangle(double width, double height)
		: m_width(width)
		, m_height(height)
	{
	}
	string ToString() const override
	{
		return "Rectangle, width: " + to_string(m_width)
			+ ", height: " + to_string(m_height)
			+ ", area: " + to_string(GetArea());
	}
	double GetArea() const override
	{
		return m_width * m_height;
	}
private:
	double m_width;
	double m_height;
};

Shape * NaiveLoadShape(istream & in)
{
	string info;
	if (!getline(in, info))
		return NULL;
	istringstream parser(info);

	string type;
	if (parser >> type)
	{
		// Конструируем фигуру нужного типа
		if (type == "circle")
		{
			double r;
			if (parser >> r)
				return new Circle(r);
		}
		else if (type == "rectangle")
		{
			double w, h;
			if (parser >> w >> h)
				return new Rectangle(w, h);
		}
	}

	return NULL;
}


void NaiveWorkWithShapes()
{
	for (;;)
	{
		cout
			<< "Type \"circle <radius>\","
			<< "\"rectangle <width> <height>\"\n"
			<< "  or anything else to skip> ";
		Shape * shape = NaiveLoadShape(cin);
		if (!shape)
		{
			break;
		}
		cout << shape->ToString() << "\n\n";
		delete shape;
	}
}

unique_ptr<Shape> LoadShape(istream & in)
{
	string info;
	if (!getline(in, info))
		return nullptr;
	istringstream parser(info);

	string type;
	if (parser >> type)
	{
		// Конструируем фигуру нужного типа
		if (type == "circle")
		{
			double r;
			if (parser >> r)
				return make_unique<Circle>(r);
		}
		else if (type == "rectangle")
		{
			double w, h;
			if (parser >> w >> h)
				return make_unique<Rectangle>(w, h);
		}
	}

	return nullptr;
}

void WorkWithShapes()
{
	for (;;)
	{
		cout 
			<< R"(Type "circle <radius>", )"
			<< R"("rectangle <width> <height>)"
			<< " or anything else to skip> ";

		auto shape = NaiveLoadShape(cin);

		if (!shape)
			break;

		cout << shape->ToString() << "\n\n";
	}
}



void SortUpToNStrings(size_t n)
{
	string * strings = new string[n];
	for (size_t i = 0; i < n; ++i)
	{
		cout << "Enter line " << (i + 1) << " of " << n << ">";
		if (!getline(cin, strings[i]))
		{
			n = i;
			break;
		}
	}

	sort(&strings[0], &strings[n]);

	for (size_t i = 0; i < n; ++i)
	{
		cout << strings[i] << "\n";
	}

	delete[] strings;
}

int main()
{
	NaiveWorkWithShapes();

	SortUpToNStrings(5);

	return 0;
}
