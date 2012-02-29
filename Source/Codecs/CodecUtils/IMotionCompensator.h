/** @file

MODULE						: IMotionCompensator

TAG								: IMC

FILE NAME					: IMotionCompensator.h

DESCRIPTION				: An interface to motion compensator implementations. 
										VectorList objects are compensated to a YUV420
										image ref.

REVISION HISTORY	:	25/09/2006 (KF): A single vector mode was added to
										the interface to allow one vector at a time to be 
										compensated. This requires an overload for the
										Compensate() method and a new method PrepareForSingleVectorMode()
										for dumping the ref into temp before compensating.

										29/09/2006 (KF): Removed the reference to VectorList objects
										to allow any structured object in the interface. The
										implementations will now define the list structure.

COPYRIGHT					: 

RESTRICTIONS			: 
===========================================================================
*/
#ifndef _IMOTIONCOMPENSATOR_H
#define _IMOTIONCOMPENSATOR_H

/*
---------------------------------------------------------------------------
	Interface definition.
---------------------------------------------------------------------------
*/
class IMotionCompensator
{
	public:
		virtual ~IMotionCompensator() {}
		
		/** Create any private memory required.
		This is primarily to encourage a two stage construction process where
		mem is not alloc in the implementation constructor. The ref points to
		contiguous mem of Lum[imgHeight*imgWidth] followed by ChrU and then ChrV
		at half the image dimensions for YUV420 planar representation.
		@return	:	1 = success, 0 = failure.
		*/
		virtual int Create(void* ref, int imgWidth,				int imgHeight, 
																	int macroBlkWidth,	int macroBlkHeight) = 0;

		/** Destroy any private mem allocations.
		Mem is dealloc without changing the size parameters.
		@return	:	none.
		*/
		virtual void Destroy(void) = 0;

		/** Reset any previous frame assumptions.
		But do not change any	mem variables or delete anything.
		@return	: none.
		*/
		virtual void Reset(void) = 0;

		/** Motion compensate to the reference.
		Do the compensation with the block sizes and image sizes defined in
		the implementation and set in Create().
		@param pMotionList	: The list of motion vectors.
		@return							: None.
		*/
		virtual void Compensate(void* pMotionList) = 0;

		/** Motion compensate a single vector to the reference.
		Do the compensation with the block sizes and image sizes defined in
		the implementation and set in Create(). PrepareForSingleVectorMode()
		should be called once per compensation session before this method.
		@param tlx	: Top left x coord of block.
		@param tly	: Top left y coord of block.
		@param mvx	: X coord of the motion vector.
		@param mvy	: Y coord of the motion vector.
		@return			: None.
		*/
		virtual void Compensate(int tlx, int tly, int mvx, int mvy) = 0;
		
		/** Prepare the ref for single motion vector compensation mode.
		Should be used to copy the ref into a temp location from which to
		do the compensation to the ref. Prevents interference and double
		compensation. Use the Invalidate() command to force compensation
    on a vector by vector basis.
		@return : none.
		*/
		virtual void PrepareForSingleVectorMode(void) = 0;
    virtual void Invalidate(void) {}

};//end IMotionEstimator.


#endif
