--
--  getVTitem.sql: get a node on its full name
--
--  Copyright (C) 2011
--  ASTRON (Netherlands Foundation for Research in Astronomy)
--  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
--
--  This program is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with this program; if not, write to the Free Software
--  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--  $Id: getVTitemList_func.sql 9567 2006-11-01 09:11:34Z overeem $
--

--
-- getVTitemRecursive (treeID, name, index, parentid)
-- 
-- Get an item on its name.
--
-- Authorisation: none
--
-- Tables: 	victemplate		read
--
-- Types:	OTDBnode
--
CREATE OR REPLACE FUNCTION getVTitemRecursive(INT4, VARCHAR(150), INT4)
  RETURNS OTDBnode AS $$
	DECLARE
		vRecord			RECORD;
		vParentName		VARCHAR(150);
		vChildName		VARCHAR(150);
		aFullName		ALIAS FOR $2;
		aParentID		ALIAS FOR $3;
		vIndex  		VARCHAR(50);
--      The database [March 2011] has parameters with index=1 and index=-1
--      that is due to a change in the code used to load the components
--		As long as that is the case we have to work with 2 index values
		vIndex2			INTEGER;

	BEGIN
	  -- strip off first part of name and try if it has an index
	  vParentName := split_part(aFullName, '.', 1);
	  vIndex := substring(vParentName from '.*\\[([0-9])\\]$');
	  IF vIndex IS NULL THEN
		vIndex := -1;
		vIndex2 := 1;
	  ELSE
		vIndex2 := vIndex;
		vParentName := substring(vParentName from '([^\\[]+)\\[.*');
	  END IF;
	  -- remember the remainder of the name
	  vChildName := substring(aFullName from '[^\\.]+\\.(.*)');

	  IF aParentID = 0 THEN
        SELECT nodeid,
               parentid,
               originid,
               name,
               index,
               leaf,
               instances,
               limits,
               ''::text
        INTO	 vRecord
        FROM   VICtemplate
        WHERE  treeID = $1
        AND    name   = vParentName
	    AND    (index = vIndex OR index = vIndex2);
	  ELSE
        SELECT nodeid,
               parentid,
               originid,
               name,
               index,
               leaf,
               instances,
               limits,
               ''::text
        INTO	 vRecord
        FROM   VICtemplate
        WHERE  treeID = $1
        AND    name   = vParentName
	    AND    (index = vIndex OR index = vIndex2)
        AND    parentid = aParentID;
	  END IF;

	  IF FOUND AND NOT vChildName IS NULL THEN
		vRecord := getVTitemRecursive($1, vChildName, vRecord.nodeid);
	  END IF;
	  RETURN vRecord;
	END;
$$ LANGUAGE plpgsql;

--
-- getVTitem (treeID, fullname)
-- 
-- Get an items on its fullname.
--
-- Authorisation: none
--
-- Tables: 	victemplate		read
--
-- Types:	OTDBnode
--
CREATE OR REPLACE FUNCTION getVTitem(INT4, VARCHAR(150))
  RETURNS OTDBnode AS $$
    DECLARE
	  vRecord	RECORD;

	BEGIN
	  vRecord := getVTitemRecursive($1, $2, 0);
	  RETURN vRecord;
	END
$$ LANGUAGE plpgsql;
