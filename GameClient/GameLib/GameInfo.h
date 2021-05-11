#pragma once

typedef struct Cell
{
	int x;
	int y;

	Cell() {}
	Cell(int _x, int _y) { x = _x; y = _y; }
	Cell(Cell& _position) { x = _position.x; y = _position.y; }

} Cell;
