/** @file

MODULE						: MeasurementTable

TAG								: MT

FILE NAME					: MeasurementTable.cpp

DESCRIPTION				: A class to hold measurement data in the form of a table
										with a set number of columns and rows.

REVISION HISTORY	:
									: 

COPYRIGHT					:

RESTRICTIONS			: 
===========================================================================
*/
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <stdio.h>
#endif

#include <stdio.h>
#include <string.h>
#include "MeasurementTable.h"

MeasurementTable::MeasurementTable(void)
{
	_columns	= 0;
	_rows			= 0;
	_ppTable	= NULL;
	_pTable		= NULL;
	_ppHead		= NULL;
	_pTitle		= NULL;
	_pType		= NULL;
}//end constructor.

MeasurementTable::~MeasurementTable(void)
{
	Destroy();
}//end destructor.

int MeasurementTable::Create(int columns, int rows)
{
	Destroy();

	_pTable = new double[columns * rows];
	if(_pTable == NULL)
		return(0);

	_ppTable = new double*[rows];
	if(_ppTable == NULL)
	{
		delete[] _pTable;
		_pTable = NULL;
		return(0);
	}//end if _ppTable...
	// Load row addresses.
	for(int i = 0; i < rows; i++)
		_ppTable[i] = &(_pTable[i * columns]);

	_columns	= columns;
	_rows			= rows;

	// Headings for each column.
	_ppHead = new char*[columns];
	if(_ppHead == NULL)
	{
		Destroy();
		return(0);
	}//end if !_ppHead...
	for(int col = 0; col < columns; col++)
	{
		_ppHead[col] = new char[80];
		if(_ppHead[col] == NULL)
		{
			Destroy();
			return(0);
		}//end if !_ppHead...
		strcpy(_ppHead[col], "");
	}//end for col...

	// File title.
	_pTitle = new char[120];
	if(_pTitle == NULL)
	{
		Destroy();
		return(0);
	}//end if !_pTitle...
	strcpy(_pTitle, "");

	_pType = new int[columns];
	if(_pType == NULL)
	{
		Destroy();
		return(0);
	}//end if !_pType...
	for(int c = 0; c < columns; c++)
		_pType[c] = INT;

	return(1);
}//end Create.

void MeasurementTable::Destroy(void)
{
	if(_ppTable != NULL)
		delete[] _ppTable;
	_ppTable = NULL;

	if(_pTable != NULL)
		delete[] _pTable;
	_pTable = NULL;

	if(_ppHead != NULL)
	{
		for(int c = 0; c < _columns; c++)
		{
			if(_ppHead[c] != NULL)
			{
				delete[] _ppHead[c];
				_ppHead[c] = NULL;
			}//end if _ppHead...
		}//end for c...
		delete[] _ppHead;
	}//end if _ppHead...
	_ppHead = NULL;

	if(_pTitle != NULL)
		delete[] _pTitle;
	_pTitle = NULL;

	if(_pType != NULL)
		delete[] _pType;
	_pType = NULL;

	_columns = 0;
	_rows    = 0;
}//end Destroy.

int	MeasurementTable::Save(char* filename, char* delimiter)
{
	int col, row;

  if(_pTable == NULL)
    return(0);

	FILE*	stream = fopen(filename, "w");
  if(stream == NULL)
    return(0);

	// File title.
	if(_pTitle != NULL)
		fprintf(stream, "%s\n", _pTitle);

	// Column headings.
	if(_ppHead != NULL)
	{
		for(col = 0; col < (_columns - 1); col++)
		{
			if(_ppHead[col] != NULL)
				fprintf(stream, "%s%s", _ppHead[col], delimiter);
		}//end for col...
		if(_ppHead[col] != NULL)
			fprintf(stream, "%s\n", _ppHead[col]);
	}//end if _ppHead...

  // Store measurement results.
  for(row = 0; row < _rows; row++)
	{
		for(col = 0; col < (_columns - 1); col++)
		{
			if(_pType[col] == INT)
				fprintf(stream, "%d%s", (int)(_ppTable[row][col]), delimiter);
			else	// DOUBLE.
				fprintf(stream, "%f%s", (double)(_ppTable[row][col]), delimiter);
		}//end for col...

		if(_pType[col] == INT)
			fprintf(stream, "%d\n", (int)(_ppTable[row][col]));
		else	// DOUBLE.
			fprintf(stream, "%f\n", (double)(_ppTable[row][col]));

	}//end for row...

	fclose(stream);

	return(1);
}//end Save.

void MeasurementTable::SetHeading(int col, const char* heading)
{
	if( (_ppHead[col] != NULL)&&(strlen(heading) < 80) )
	{
		strcpy(_ppHead[col], heading);
	}//end if _ppHead...
}//end SetHeading.

void MeasurementTable::SetTitle(const char* title)
{
	if( (_pTitle != NULL)&&(strlen(title) < 80) )
	{
		strcpy(_pTitle, title);
	}//end if _pTitle...
}//end SetTitle.

void MeasurementTable::SetDataType(int col, int type)
{
	if( (_pType != NULL)&&(type <= DOUBLE) )
	{
		_pType[col] = type;
	}//end if _pType...
}//end SetDataType.


