/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-  */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef _MDB_
#include "mdb.h"
#endif

#ifndef _MORK_
#include "mork.h"
#endif

#ifndef _MORKNODE_
#include "morkNode.h"
#endif

#ifndef _MORKOBJECT_
#include "morkObject.h"
#endif

#ifndef _MORKENV_
#include "morkEnv.h"
#endif

#ifndef _MORKCELLOBJECT_
#include "morkCellObject.h"
#endif

#ifndef _MORKROWOBJECT_
#include "morkRowObject.h"
#endif

#ifndef _MORKROW_
#include "morkRow.h"
#endif

#ifndef _MORKCELL_
#include "morkCell.h"
#endif

#ifndef _MORKSTORE_
#include "morkStore.h"
#endif

//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789

// ````` ````` ````` ````` `````
// { ===== begin morkNode interface =====

/*public virtual*/ void
morkCellObject::CloseMorkNode(morkEnv* ev) // CloseCellObject() only if open
{
  if ( this->IsOpenNode() )
  {
    this->MarkClosing();
    this->CloseCellObject(ev);
    this->MarkShut();
  }
}

/*public virtual*/
morkCellObject::~morkCellObject() // assert CloseCellObject() executed earlier
{
  CloseMorkNode(mMorkEnv);
  MORK_ASSERT(mCellObject_Row==0);
}

/*public non-poly*/
morkCellObject::morkCellObject(morkEnv* ev, const morkUsage& inUsage,
  nsIMdbHeap* ioHeap, morkRow* ioRow, morkCell* ioCell,
  mork_column inCol, mork_pos inPos)
: morkObject(ev, inUsage, ioHeap, morkColor_kNone, (morkHandle*) 0)
, mCellObject_RowObject( 0 )
, mCellObject_Row( 0 )
, mCellObject_Cell( 0 )
, mCellObject_Col( inCol )
, mCellObject_RowSeed( 0 )
, mCellObject_Pos( (mork_u2) inPos )
{
  if ( ev->Good() )
  {
    if ( ioRow && ioCell )
    {
      if ( ioRow->IsRow() )
      {
        morkStore* store = ioRow->GetRowSpaceStore(ev);
        if ( store )
        {
          morkRowObject* rowObj = ioRow->AcquireRowObject(ev, store);
          if ( rowObj )
          {
            mCellObject_Row = ioRow;
            mCellObject_Cell = ioCell;
            mCellObject_RowSeed = ioRow->mRow_Seed;

            // morkRowObject::SlotStrongRowObject(rowObj, ev,
            //  &mCellObject_RowObject);

            mCellObject_RowObject = rowObj; // assume control of strong ref
          }
          if ( ev->Good() )
            mNode_Derived = morkDerived_kCellObject;
        }
      }
      else
        ioRow->NonRowTypeError(ev);
    }
    else
      ev->NilPointerError();
  }
}

NS_IMPL_ISUPPORTS_INHERITED(morkCellObject, morkObject, nsIMdbCell)

/*public non-poly*/ void
morkCellObject::CloseCellObject(morkEnv* ev) // called by CloseMorkNode();
{
    if ( this->IsNode() )
    {
      NS_RELEASE(mCellObject_RowObject);
      mCellObject_Row = 0;
      mCellObject_Cell = 0;
      mCellObject_RowSeed = 0;
      this->CloseObject(ev);
      this->MarkShut();
    }
    else
      this->NonNodeError(ev);
}

// } ===== end morkNode methods =====
// ````` ````` ````` ````` `````

mork_bool
morkCellObject::ResyncWithRow(morkEnv* ev)
{
  morkRow* row = mCellObject_Row;
  mork_pos pos = 0;
  morkCell* cell = row->GetCell(ev, mCellObject_Col, &pos);
  if ( cell )
  {
    mCellObject_Pos = (mork_u2) pos;
    mCellObject_Cell = cell;
    mCellObject_RowSeed = row->mRow_Seed;
  }
  else
  {
    mCellObject_Cell = 0;
    this->MissingRowColumnError(ev);
  }
  return ev->Good();
}

morkAtom*
morkCellObject::GetCellAtom(morkEnv* ev) const
{
  morkCell* cell = mCellObject_Cell;
  if ( cell )
    return cell->GetAtom();
  else
    this->NilCellError(ev);

  return (morkAtom*) 0;
}

/*static*/ void
morkCellObject::WrongRowObjectRowError(morkEnv* ev)
{
  ev->NewError("mCellObject_Row != mCellObject_RowObject->mRowObject_Row");
}

/*static*/ void
morkCellObject::NilRowError(morkEnv* ev)
{
  ev->NewError("nil mCellObject_Row");
}

/*static*/ void
morkCellObject::NilRowObjectError(morkEnv* ev)
{
  ev->NewError("nil mCellObject_RowObject");
}

/*static*/ void
morkCellObject::NilCellError(morkEnv* ev)
{
  ev->NewError("nil mCellObject_Cell");
}

/*static*/ void
morkCellObject::NonCellObjectTypeError(morkEnv* ev)
{
  ev->NewError("non morkCellObject");
}

/*static*/ void
morkCellObject::MissingRowColumnError(morkEnv* ev)
{
  ev->NewError("mCellObject_Col not in mCellObject_Row");
}

nsIMdbCell*
morkCellObject::AcquireCellHandle(morkEnv* ev)
{
  nsIMdbCell* outCell = this;
  NS_ADDREF(outCell);
  return outCell;
}


morkEnv*
morkCellObject::CanUseCell(nsIMdbEnv* mev, mork_bool inMutable,
                           nsresult* outErr, morkCell** outCell)
{
  morkEnv* outEnv = 0;
  morkCell* cell = 0;
  morkEnv* ev = morkEnv::FromMdbEnv(mev);
  if ( ev )
  {
    if ( IsCellObject() )
    {
      if ( IsMutable() || !inMutable )
      {
        morkRowObject* rowObj = mCellObject_RowObject;
        if ( rowObj )
        {
          morkRow* row = mCellObject_Row;
          if ( row )
          {
            if ( rowObj->mRowObject_Row == row )
            {
              mork_u2 oldSeed = mCellObject_RowSeed;
              if ( row->mRow_Seed == oldSeed || ResyncWithRow(ev) )
              {
                cell = mCellObject_Cell;
                if ( cell )
                {
                  outEnv = ev;
                }
                else
                  NilCellError(ev);
              }
            }
            else
              WrongRowObjectRowError(ev);
          }
          else
            NilRowError(ev);
        }
        else
          NilRowObjectError(ev);
      }
      else
        NonMutableNodeError(ev);
    }
    else
      NonCellObjectTypeError(ev);
  }
  *outErr = ev->AsErr();
  MORK_ASSERT(outEnv);
  *outCell = cell;

  return outEnv;
}

// { ----- begin attribute methods -----
NS_IMETHODIMP morkCellObject::SetBlob(nsIMdbEnv* /* mev */,
  nsIMdbBlob* /* ioBlob */)
{
  NS_ASSERTION(false, "not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
} // reads inBlob slots

// when inBlob is in the same suite, this might be fastest cell-to-cell

NS_IMETHODIMP morkCellObject::ClearBlob( // make empty (so content has zero length)
  nsIMdbEnv*  /* mev */)
{
  NS_ASSERTION(false, "not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
  // remember row->MaybeDirtySpaceStoreAndRow();
}
// clearing a yarn is like SetYarn() with empty yarn instance content

NS_IMETHODIMP morkCellObject::GetBlobFill(nsIMdbEnv* mev,
  mdb_fill* outFill)
// Same value that would be put into mYarn_Fill, if one called GetYarn()
// with a yarn instance that had mYarn_Buf==nil and mYarn_Size==0.
{
  NS_ASSERTION(false, "not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}  // size of blob

NS_IMETHODIMP morkCellObject::SetYarn(nsIMdbEnv* mev,
  const mdbYarn* inYarn)
{
  nsresult outErr = NS_OK;
  morkCell* cell = 0;
  morkEnv* ev = this->CanUseCell(mev, /*inMutable*/ morkBool_kTrue,
    &outErr, &cell);
  if ( ev )
  {
    morkRow* row = mCellObject_Row;
    if ( row )
    {
      morkStore* store = row->GetRowSpaceStore(ev);
      if ( store )
      {
        cell->SetYarn(ev, inYarn, store);
        if ( row->IsRowClean() && store->mStore_CanDirty )
          row->MaybeDirtySpaceStoreAndRow();
      }
    }
    else
      ev->NilPointerError();

    outErr = ev->AsErr();
  }

  return outErr;
}   // reads from yarn slots
// make this text object contain content from the yarn's buffer

NS_IMETHODIMP morkCellObject::GetYarn(nsIMdbEnv* mev,
  mdbYarn* outYarn)
{
  nsresult outErr = NS_OK;
  morkCell* cell = 0;
  morkEnv* ev = this->CanUseCell(mev, /*inMutable*/ morkBool_kTrue,
    &outErr, &cell);
  if ( ev )
  {
    morkAtom* atom = cell->GetAtom();
    morkAtom::GetYarn(atom, outYarn);
    outErr = ev->AsErr();
  }

  return outErr;
}  // writes some yarn slots
// copy content into the yarn buffer, and update mYarn_Fill and mYarn_Form

NS_IMETHODIMP morkCellObject::AliasYarn(nsIMdbEnv* mev,
  mdbYarn* outYarn)
{
  nsresult outErr = NS_OK;
  morkCell* cell = 0;
  morkEnv* ev = this->CanUseCell(mev, /*inMutable*/ morkBool_kTrue,
    &outErr, &cell);
  if ( ev )
  {
    morkAtom* atom = cell->GetAtom();
    morkAtom::AliasYarn(atom, outYarn);
    outErr = ev->AsErr();
  }

  return outErr;
} // writes ALL yarn slots

// } ----- end attribute methods -----

// } ===== end nsIMdbBlob methods =====

// { ===== begin nsIMdbCell methods =====

// { ----- begin attribute methods -----
NS_IMETHODIMP morkCellObject::SetColumn(nsIMdbEnv* mev, mdb_column inColumn)
{
  NS_ASSERTION(false, "not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
  // remember row->MaybeDirtySpaceStoreAndRow();
}

NS_IMETHODIMP morkCellObject::GetColumn(nsIMdbEnv* mev, mdb_column* outColumn)
{
  nsresult outErr = NS_OK;
  mdb_column col = 0;
  morkCell* cell = 0;
  morkEnv* ev = this->CanUseCell(mev, /*inMutable*/ morkBool_kTrue,
    &outErr, &cell);
  if ( ev )
  {
    col = mCellObject_Col;
    outErr = ev->AsErr();
  }
  if ( outColumn )
    *outColumn = col;
  return outErr;
}

NS_IMETHODIMP morkCellObject::GetCellInfo(  // all cell metainfo except actual content
  nsIMdbEnv* mev,
  mdb_column* outColumn,           // the column in the containing row
  mdb_fill*   outBlobFill,         // the size of text content in bytes
  mdbOid*     outChildOid,         // oid of possible row or table child
  mdb_bool*   outIsRowChild)  // nonzero if child, and a row child
// Checking all cell metainfo is a good way to avoid forcing a large cell
// in to memory when you don't actually want to use the content.
{
  NS_ASSERTION(false, "not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}


NS_IMETHODIMP morkCellObject::GetRow(nsIMdbEnv* mev, // parent row for this cell
  nsIMdbRow** acqRow)
{
  nsresult outErr = NS_OK;
  nsIMdbRow* outRow = 0;
  morkCell* cell = 0;
  morkEnv* ev = this->CanUseCell(mev, /*inMutable*/ morkBool_kTrue,
    &outErr, &cell);
  if ( ev )
  {
    outRow = mCellObject_RowObject->AcquireRowHandle(ev);

    outErr = ev->AsErr();
  }
  if ( acqRow )
    *acqRow = outRow;
  return outErr;
}

NS_IMETHODIMP morkCellObject::GetPort(nsIMdbEnv* mev, // port containing cell
  nsIMdbPort** acqPort)
{
  nsresult outErr = NS_OK;
  nsIMdbPort* outPort = 0;
  morkCell* cell = 0;
  morkEnv* ev = this->CanUseCell(mev, /*inMutable*/ morkBool_kTrue,
    &outErr, &cell);
  if ( ev )
  {
    if ( mCellObject_Row )
    {
      morkStore* store = mCellObject_Row->GetRowSpaceStore(ev);
      if ( store )
        outPort = store->AcquireStoreHandle(ev);
    }
    else
      ev->NilPointerError();

    outErr = ev->AsErr();
  }
  if ( acqPort )
    *acqPort = outPort;
  return outErr;
}
// } ----- end attribute methods -----

// { ----- begin children methods -----
NS_IMETHODIMP morkCellObject::HasAnyChild( // does cell have a child instead of text?
  nsIMdbEnv* mev,
  mdbOid* outOid,  // out id of row or table (or unbound if no child)
  mdb_bool* outIsRow) // nonzero if child is a row (rather than a table)
{
  nsresult outErr = NS_OK;
  mdb_bool isRow = morkBool_kFalse;
  outOid->mOid_Scope = 0;
  outOid->mOid_Id = morkId_kMinusOne;
  morkCell* cell = 0;
  morkEnv* ev = this->CanUseCell(mev, /*inMutable*/ morkBool_kTrue,
    &outErr, &cell);
  if ( ev )
  {
    morkAtom* atom = GetCellAtom(ev);
    if ( atom )
    {
      isRow = atom->IsRowOid();
      if ( isRow || atom->IsTableOid() )
        *outOid = ((morkOidAtom*) atom)->mOidAtom_Oid;
    }

    outErr = ev->AsErr();
  }
  if ( outIsRow )
    *outIsRow = isRow;

  return outErr;
}

NS_IMETHODIMP morkCellObject::GetAnyChild( // access table of specific attribute
  nsIMdbEnv* mev, // context
  nsIMdbRow** acqRow, // child row (or null)
  nsIMdbTable** acqTable) // child table (or null)
{
  NS_ASSERTION(false, "not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}


NS_IMETHODIMP morkCellObject::SetChildRow( // access table of specific attribute
  nsIMdbEnv* mev, // context
  nsIMdbRow* ioRow)
{
  NS_ASSERTION(false, "not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
} // inRow must be bound inside this same db port

NS_IMETHODIMP morkCellObject::GetChildRow( // access row of specific attribute
  nsIMdbEnv* mev, // context
  nsIMdbRow** acqRow) // acquire child row (or nil if no child)
{
  NS_ASSERTION(false, "not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}


NS_IMETHODIMP morkCellObject::SetChildTable( // access table of specific attribute
  nsIMdbEnv* mev, // context
  nsIMdbTable* inTable) // table must be bound inside this same db port
{
  NS_ASSERTION(false, "not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
  // remember row->MaybeDirtySpaceStoreAndRow();
}

NS_IMETHODIMP morkCellObject::GetChildTable( // access table of specific attribute
  nsIMdbEnv* mev, // context
  nsIMdbTable** acqTable) // acquire child tabdle (or nil if no chil)
{
  NS_ASSERTION(false, "not implemented");
  return NS_ERROR_NOT_IMPLEMENTED;
}
// } ----- end children methods -----

// } ===== end nsIMdbCell methods =====


//3456789_123456789_123456789_123456789_123456789_123456789_123456789_123456789
