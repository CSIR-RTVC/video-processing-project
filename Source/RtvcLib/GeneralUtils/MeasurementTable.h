/** @file

MODULE				: MeasurementTable

TAG						: MT

FILE NAME			: MeasurementTable.h

DESCRIPTION		: A class to hold measurement data in the form of a table
								with a set number of columns and rows.

COPYRIGHT			: (c)CSIR 2007-2013 all rights resevered

LICENSE				: Software License Agreement (BSD License)

RESTRICTIONS	: Redistribution and use in source and binary forms, with or without 
								modification, are permitted provided that the following conditions 
								are met:

								* Redistributions of source code must retain the above copyright notice, 
								this list of conditions and the following disclaimer.
								* Redistributions in binary form must reproduce the above copyright notice, 
								this list of conditions and the following disclaimer in the documentation 
								and/or other materials provided with the distribution.
								* Neither the name of the CSIR nor the names of its contributors may be used 
								to endorse or promote products derived from this software without specific 
								prior written permission.

								THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
								"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
								LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
								A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
								CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
								EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
								PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
								PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
								LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
								NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
								SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
	int		Save(char* filename, char* delimiter, int withHeadings);

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
