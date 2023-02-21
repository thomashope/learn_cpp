#include <iostream>
#include <algorithm>

class Widget
{
public:

	// user defined constructor
	Widget(int i, float f)
	{
		std::cout << "Constructor: " << i << " " << f << std::endl;

		m_myInt = i;
		m_myFloat = new float(f);
	}

	~Widget()
	{
		delete m_myFloat;
	}

	// copy constructor
	Widget(const Widget& other)
	{
		std::cout << "Copy Constructor" << std::endl;

		m_myInt = other.m_myInt;
		m_myFloat = new float;
		*m_myFloat = *other.m_myFloat;
	}

	// move constructor
	Widget(Widget&& other)
	{
		std::cout << "Move Constructor" << std::endl;

		m_myInt = other.m_myInt;
		std::swap(m_myFloat, other.m_myFloat);
	}

	// copy assignment operator
	Widget& operator = (const Widget& other)
	{
		std::cout << "Copy Assignment Operator" << std::endl;

		m_myInt = other.m_myInt;
		m_myFloat = new float;
		*m_myFloat = *other.m_myFloat;

		return *this;
	}

	// move assignment oerator
	Widget& operator = (Widget&& other)
	{
		std::cout << "Move Assignment Operator" << std::endl;

		m_myInt = other.m_myInt;
		std::swap(m_myFloat, other.m_myFloat);
		return *this;
	}

private:
	int 	m_myInt;
	float* 	m_myFloat;
};

Widget factory(int i, float f)
{
	return Widget(i, f);
}

int main()
{
	Widget a(1, 2.0f); // constructor
	Widget b(2, 3.0f); // constructor
	
	Widget c = a; // copy constructor
	a = b; // copy assignment operator

	Widget d = std::move(a); // move constructor, a is now garbage?
	c = std::move(b); // move assignment operator, b is now garbave?

	Widget e = factory(8, 9); // constructor inside factor(), copy is elided?
	e = factory(10, 11); // move assignment operator, why?

	return 0;
}