/** @file

MODULE						: MeasurementTable

TAG								: MT

FILE NAME					: MeasurementTable.h

DESCRIPTION				: A class to hold measurement data in the form of a table
										with a set number of columns and rows.

REVISION HISTORY	:
									: 

COPYRIGHT					:

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _MEASUREMENTTABLE_H
#define _MEASUREMENTTABLE_H

#pragma once

/*
---------------------------------------------------------------------------
	Class definition.
---------------------------------------------------------------------------
*/
class MeasurementTable
{
public:
	MeasurementTable(void);
	virtual ~MeasurementTable();

// Setup and interface methods.
public:
	int		Create(int columns, int rows);
	void	Destroy(void);
	int		Save(char* filename, char* delimiter);

	virtual void		WriteItem(int col, int row, int x)		{ _ppTable[row][col] = (double)x; }
	virtual void		WriteItem(int col, int row, double x) { _ppTable[row][col] = x; }
	virtual double	ReadItem(int col, int row) { return(_ppTable[row][col]); }
	void						SetTitle(const char* title);
	void						SetHeading(int col, const char* heading);
	void						SetDataType(int col, int type);

// Class constants.
public:
	static const int INT		= 0;
	static const int DOUBLE = 1;

// Common members.
protected:
	int				_columns;
	int				_rows;
	double**	_ppTable;
	double*		_pTable;
	char*			_pTitle;
	char**		_ppHead;
	int*			_pType;

};// end class MeasurementTable.

#endif	// _MEASUREMENTTABLE_H
