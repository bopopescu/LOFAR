// file: fwTreeViewTest.ctl (old name: tree/libTreeTest.ctl)
//
// contents: library used by following panels:
// tree/treeViewTest1.pnl, tree/treeViewTest2.pnl, tree/treeTest3.pnl
// Those panels are examples of use of tree/treeView.pnl panel and
// of tree/treeView.ctl library.
//
// date            author        version               modification
// April 14, 00    Ph Gras         v1.0      creation 
// April 9, 01     Ph. Gras        v2.0      1. function renaming in
//                                           order to include tree package 
//                                           in JCOP framework.
//                                           2. making comment compliante with
//                                           doc++.
//------------------------------------------------------------------------------

/** @name treeViewTest.ctl library
 * Library used by following panels:
 * tree/treeViewTest1.pnl, tree/treeViewTest2.pnl, tree/treeTest3.pnl
 * Those panels are examples of use of tree/treeView.pnl panel and
 * of tree/treeView.ctl library.
 * @author Ph. Gras, CERN/EP - University of Karlsruhe
 * @version 2.0
 */ 
//@{
/** Function passed as $sOnInit parameter to 
 * tree/treeView.pnl panel.
 * this function constructs a tree inspired
 * of some filesystem.
 */
fwTreeView_TestInit(){
	fwTreeView_appendNode("/", "1,048,576 ko", 0, 0);
	fwTreeView_appendNode("bin/", "1 ko", 0, 1);
	fwTreeView_appendNode("etc/", "3,875 ko", 0, 1);
	fwTreeView_appendNode("usr/", "385,181 ko", 0, 1);
	fwTreeView_appendNode("bin/", "19,381 ko", 0, 2);
	fwTreeView_appendNode("lib/", "32,824 ko", 0, 2);
	fwTreeView_appendNode("sbin/", "11,053 ko", 0, 2);
	fwTreeView_appendNode("var/", "66,537 ko", 0, 1);
	fwTreeView_appendNode("adm/", "17, 383 ko", 0, 2);
	fwTreeView_appendNode("log/", "3,819 ko", 0, 2);
	fwTreeView_appendNode("sbin/", "5,603 ko", 0, 1);
	id = -1;
}

/** Function passed to the $sOnSelect parameter
 * of tree/treeView.pnl panel.
 * This function display some text that was attached to the
 * node by the fwTreeView_TestInit function.
 */		
fwTreeView_TestSelect(unsigned pos){
  dyn_anytype node = fwTreeView_getNode(pos);

  setValue("dir", "text", node[5]);

  id = -1;
}

//@} end of treeViewTest.ctl library
