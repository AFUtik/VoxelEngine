#ifndef NODE_HPP
#define NODE_HPP

class Node {
public:
	virtual void ready() {};
	virtual void process(double dt) {};
	virtual void physicsProcess(double dt) {};
};

#endif