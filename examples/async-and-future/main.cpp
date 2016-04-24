#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <future>
#include <fstream>
#include <iostream>
#include <iterator>
#include <functional>

using namespace std;
constexpr auto M_PI = 3.14159265358979323846;


class Shape
{
public:
	virtual double GetArea()const = 0;
	virtual string ToString()const = 0;
	virtual ~Shape() = default;
};

class Circle : public Shape
{
public:
	double GetArea()const final
	{
		return M_PI * m_radius * m_radius;
	}
	string ToString()const override
	{
		return "circle";
	}
private:
	double m_radius = 42;
};

void Auto()
{
	vector<string> animals = { "cat", "dog", "wolf", "rabbit" };

	{
		vector<string>::iterator it = find(animals.begin(), animals.end(), "rabbit");
	}

	{
		auto it = find(animals.begin(), animals.end(), "rabbit");
	}
}

void Lambda()
{
	vector<string> animals = { "cat", "dog", "wolf", "rabbit" };

	sort(animals.begin(), animals.end());
}



void RawStringLiterals()
{
	string s =
		"<document name=\"Report1\">\n"
		"\t<item id=\"item1\" title=\"Chapter 1\"/>\n"
		"</document>";

	string q =
		R"(<document name="Report1">
	<item id="item1" title="Chapter 1"/>
</document>)";
}

void AsyncAndFuture()
{
	auto fileName = "main.cpp";
	auto f = async(launch::async, [=] {
		ifstream in;
		in.exceptions(ios_base::badbit);
		in.open(fileName);
		in >> noskipws;
		return string(istream_iterator<char>(in), istream_iterator<char>());
	});

	/* Do some calculations*/

	auto content = f.get();
	cout << content << endl;
}


class Duck
{
public:
	void Quack() const
	{
		m_quackBehavior();
	}
	void Fly()
	{
		m_flyBehavior();
	}
protected:
	function<void()> m_flyBehavior;
	function<void()> m_quackBehavior;
};

void FlyWithWings()
{
	cout << "I'm flying\n";
}

class MallardDuck : public Duck
{
public:
	MallardDuck()
	{
		m_quackBehavior = [] {cout << "Quack-Quack\n"; };
		m_flyBehavior = FlyWithWings;
	}
};



int main()
{
	AsyncAndFuture();
}
