#pragma once
#include <algorithm>
#include <limits>
#include <list>
#include <memory>
#include <set>
#include <vector>

using namespace std;

typedef double CoordinateType;
static const CoordinateType infinity = numeric_limits<CoordinateType>::max();

typedef vector<CoordinateType> Coordinates;

enum class DimensionType : Coordinates::size_type {};

DimensionType next(DimensionType dimension, DimensionType maxDimension);

class Point {
public:
	Point(size_t id, const Coordinates & location):
		_id(id),
		_location(location) {
	}

	Point(size_t id, Coordinates && location) :
		_id(id),
		_location(move(location)) {
	}

	CoordinateType Component(DimensionType dimension) const {
		return _location[static_cast<Coordinates::size_type>(dimension)];
	}

	DimensionType Dimension() const {
		return DimensionType(_location.size());
	}

private:
	size_t _id;
	Coordinates _location;
};

class DimensionComparator {
public:
	explicit DimensionComparator(DimensionType dimension) :
		_dim(dimension) {
	}

	DimensionComparator(const DimensionComparator &) = default;
	DimensionComparator(DimensionComparator &&) = default;


	bool operator()(const Point & p1, const Point & p2) const {
		return p1.Component(_dim) < p2.Component(_dim);
	}
private:
	const DimensionType _dim;
};

class Bounds {
public:
	Bounds(const Coordinates & mins, const Coordinates & maxs) :
		_mins(mins),
		_maxs(maxs) {
	}

	Bounds(Coordinates && mins, Coordinates && maxs) :
		_mins(move(mins)),
		_maxs(move(maxs)) {
	}

	Bounds(const Bounds & bounds) = default;
	Bounds(Bounds && bounds) = default;

	Bounds Split(DimensionType dimension, CoordinateType splitValue);

private:
	Coordinates _mins;
	Coordinates _maxs;
};

class SpatialBranch {
public:
	virtual void Add(Point && point) = 0;
	virtual bool MustSplit() const = 0;

	virtual size_t Size() const = 0;
	virtual size_t NumLeaves() const = 0;
	virtual size_t Depth() const = 0;
	virtual size_t MaxItemsPerLeaf() const = 0;
};

class SpatialLeaf: public SpatialBranch {
public:
	SpatialLeaf(Bounds && bounds, size_t maxItems):
		_bounds(move(bounds)),
		_maxItems(maxItems) {
	}

	virtual void Add(Point && point) override;
	virtual bool MustSplit() const override {
		return _points.size() > _maxItems;
	}

	virtual size_t Size() const {
		return _points.size();
	}
	virtual size_t NumLeaves() const override {
		return 1;
	}
	virtual size_t Depth() const override {
		return 1;
	}
	virtual size_t MaxItemsPerLeaf() const override {
		return Size();
	}

	SpatialLeaf Split(DimensionType dimension, CoordinateType & splitValue);

private:
	Bounds _bounds;
	size_t _maxItems;
	list<Point> _points;
};

class SpatialTree : public SpatialBranch {
public:
	SpatialTree(DimensionType splitDimension, CoordinateType splitValue);

	virtual void Add(Point && point) override;
	virtual bool MustSplit() const override {
		return false;
	}

	virtual size_t Size() const {
		return _lb->Size() + _ub->Size();
	}
	virtual size_t NumLeaves() const override {
		return _lb->NumLeaves() + _ub->NumLeaves();
	}
	virtual size_t Depth() const override {
		return max(_lb->Depth(), _ub->Depth()) + 1;
	}
	virtual size_t MaxItemsPerLeaf() const override {
		return max(_lb->MaxItemsPerLeaf(), _ub->MaxItemsPerLeaf());
	}

private:
	DimensionType _splitDimension;
	CoordinateType _splitValue;
	unique_ptr<SpatialBranch> _lb;
	unique_ptr<SpatialBranch> _ub;
	
};

