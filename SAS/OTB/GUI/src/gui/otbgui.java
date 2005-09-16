/*
 * otbgui.java
 *
 * Created on April 26, 2005, 2:09 PM
 */

package gui;

import java.awt.List;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.rmi.Naming;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Iterator;
import javax.swing.JFileChooser;
import javax.swing.tree.*;


/**
 *
 * @author  Alex Gerdes & Arthur Coolen
 */


public class otbgui extends javax.swing.JFrame {
    
     // SettingsDialog
    static String RMIServerName   = "dop32.astron.nl";
    static String RMIServerPort   = "10099";
    static String RMIRegistryName = "OTDBRMIServer";
    static String OTDBUserName    = "";
    static String OTDBPassword    = "";
    static String OTDBDBName      = "";   
    

    DefaultMutableTreeNode root = new DefaultMutableTreeNode();
    DefaultMutableTreeNode aNode= new DefaultMutableTreeNode();
    
    static ArrayList<String []> people;
    String [] data;
    String [] treeLine;
    boolean isTreeFilled = false;
    
    /** Creates new form otbgui */
    public otbgui() {
        initComponents();
    }

    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */   
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        jTabbedPane2 = new javax.swing.JTabbedPane();
        TreePanel = new javax.swing.JPanel();
        TreeTable = new javax.swing.JTable();
        TreeApplyButton = new javax.swing.JButton();
        BrowsePanel = new javax.swing.JPanel();
        treeScrollPane = new javax.swing.JScrollPane();
        departmentTree = new javax.swing.JTree(root);
        treeTabbedPane = new javax.swing.JTabbedPane();
        NodePanel = new javax.swing.JPanel();
        NodeIndexLabel = new javax.swing.JLabel();
        jLabel5 = new javax.swing.JLabel();
        NodeLimitsLabel = new javax.swing.JLabel();
        NodeDescriptionLabel = new javax.swing.JLabel();
        NodeIndexText = new javax.swing.JTextField();
        NodeInstancesText = new javax.swing.JTextField();
        NodeDescriptionText = new javax.swing.JTextField();
        NodeLimitsText = new javax.swing.JTextField();
        NodeOkButton = new javax.swing.JButton();
        NodeCancelButton = new javax.swing.JButton();
        NodeApplyButton = new javax.swing.JButton();
        NodeNameText = new javax.swing.JTextField();
        NodeNameLabel = new javax.swing.JLabel();
        jScrollPane2 = new javax.swing.JScrollPane();
        ParamPanel = new javax.swing.JPanel();
        ParamNameLabel = new javax.swing.JLabel();
        ParamIndexLabel = new javax.swing.JLabel();
        ParamTypeLabel = new javax.swing.JLabel();
        ParamUnitLabel = new javax.swing.JLabel();
        ParamPruningLabel = new javax.swing.JLabel();
        ParamValMomentLabel = new javax.swing.JLabel();
        ParamRTmodLabel = new javax.swing.JLabel();
        ParamLimitsLabel = new javax.swing.JLabel();
        ParamDescriptionLabel = new javax.swing.JLabel();
        ParamNameText = new javax.swing.JTextField();
        ParamIndexText = new javax.swing.JTextField();
        ParamTypeSelection = new javax.swing.JComboBox();
        ParamUnitSelection = new javax.swing.JComboBox();
        ParamPruningText = new javax.swing.JTextField();
        ParamValMomentText = new javax.swing.JTextField();
        ParamRTmodText = new javax.swing.JTextField();
        ParamLimitsText = new javax.swing.JTextField();
        ParamDescriptionText = new javax.swing.JTextField();
        ParamOkButton = new javax.swing.JButton();
        ParamCancelButton = new javax.swing.JButton();
        ParamApplyButton = new javax.swing.JButton();
        ValuePanel = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        jTable1 = new javax.swing.JTable();
        jPanel1 = new javax.swing.JPanel();
        LogNameLabel = new javax.swing.JLabel();
        LogParamnameText = new javax.swing.JTextField();
        ControlPanel = new javax.swing.JPanel();
        CtrlInstButton = new javax.swing.JButton();
        CtrlDeleteButton = new javax.swing.JButton();
        CtrlCopyButton = new javax.swing.JButton();
        CtrlChangeTreeStateButton = new javax.swing.JButton();
        infoPanel = new javax.swing.JPanel();
        SelectedLabel = new javax.swing.JLabel();
        selectedTreeTextField = new javax.swing.JTextField();
        jMenuBar1 = new javax.swing.JMenuBar();
        jMenuFile = new javax.swing.JMenu();
        jMenuFileExit = new javax.swing.JMenuItem();
        MenuInput = new javax.swing.JMenu();
        jMenuInputFile = new javax.swing.JMenuItem();
        jMenuInputOTDB = new javax.swing.JMenuItem();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setMaximizedBounds(new java.awt.Rectangle(0, 0, 1000, 400));
        setName("mainFrame");
        jTabbedPane2.setMinimumSize(new java.awt.Dimension(100, 300));
        jTabbedPane2.setPreferredSize(new java.awt.Dimension(100, 300));
        TreePanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        TreePanel.setToolTipText("Search on one or more of the given constraints");
        TreePanel.setMinimumSize(new java.awt.Dimension(500, 300));
        TreePanel.setPreferredSize(new java.awt.Dimension(500, 300));
        TreeTable.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {
                {null, null, null, null, null, null, null, null},
                {null, null, null, null, null, null, null, null},
                {null, null, null, null, null, null, null, null},
                {null, null, null, null, null, null, null, null},
                {null, null, null, null, null, null, null, null},
                {null, null, null, null, null, null, null, null},
                {null, null, null, null, null, null, null, null},
                {null, null, null, null, null, null, null, null},
                {null, null, null, null, null, null, null, null},
                {null, null, null, null, null, null, null, null}
            },
            new String [] {
                "TreeID", "Creator", "CreationDate", "Classification", "Type", "State", "Start Time", "Stop Time"
            }
        ) {
            Class[] types = new Class [] {
                java.lang.String.class, java.lang.String.class, java.lang.String.class, java.lang.String.class, java.lang.String.class, java.lang.String.class, java.lang.String.class, java.lang.String.class
            };
            boolean[] canEdit = new boolean [] {
                false, false, false, false, false, false, false, false
            };

            public Class getColumnClass(int columnIndex) {
                return types [columnIndex];
            }

            public boolean isCellEditable(int rowIndex, int columnIndex) {
                return canEdit [columnIndex];
            }
        });
        TreePanel.add(TreeTable, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 80, 740, 160));

        TreeApplyButton.setText("Apply");
        TreePanel.add(TreeApplyButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(20, 260, -1, -1));

        jTabbedPane2.addTab("TreeList", null, TreePanel, "change tree search");

        BrowsePanel.setLayout(new java.awt.BorderLayout());

        departmentTree.setAutoscrolls(true);
        departmentTree.setMinimumSize(new java.awt.Dimension(25, 72));
        departmentTree.setShowsRootHandles(true);
        departmentTree.setToggleClickCount(1);
        departmentTree.addTreeSelectionListener(new javax.swing.event.TreeSelectionListener() {
            public void valueChanged(javax.swing.event.TreeSelectionEvent evt) {
                TreeSelectionListener(evt);
            }
        });

        treeScrollPane.setViewportView(departmentTree);

        BrowsePanel.add(treeScrollPane, java.awt.BorderLayout.WEST);

        NodePanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        NodeIndexLabel.setText("Index");
        NodePanel.add(NodeIndexLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 35, -1, -1));

        jLabel5.setText("Instances");
        NodePanel.add(jLabel5, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 60, -1, -1));

        NodeLimitsLabel.setText("Limits");
        NodePanel.add(NodeLimitsLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 85, -1, -1));

        NodeDescriptionLabel.setText("Description");
        NodePanel.add(NodeDescriptionLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 110, -1, -1));

        NodeIndexText.setText("None");
        NodeIndexText.setEnabled(false);
        NodePanel.add(NodeIndexText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 35, -1, -1));

        NodeInstancesText.setText("None");
        NodePanel.add(NodeInstancesText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 60, -1, -1));

        NodeDescriptionText.setText("None");
        NodePanel.add(NodeDescriptionText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 110, -1, -1));

        NodeLimitsText.setText("None");
        NodePanel.add(NodeLimitsText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 85, -1, -1));

        NodeOkButton.setText("Ok");
        NodePanel.add(NodeOkButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 250, -1, -1));

        NodeCancelButton.setText("Cancel");
        NodePanel.add(NodeCancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 250, -1, -1));

        NodeApplyButton.setText("Apply");
        NodePanel.add(NodeApplyButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(160, 250, -1, -1));

        NodeNameText.setText("None");
        NodeNameText.setEnabled(false);
        NodePanel.add(NodeNameText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 10, -1, -1));

        NodeNameLabel.setText("Name");
        NodePanel.add(NodeNameLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 10, -1, -1));

        NodePanel.add(jScrollPane2, new org.netbeans.lib.awtextra.AbsoluteConstraints(70, 310, -1, -1));

        treeTabbedPane.addTab("Node", null, NodePanel, "Node Page");

        ParamPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        ParamNameLabel.setText("Name");
        ParamPanel.add(ParamNameLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 10, -1, -1));

        ParamIndexLabel.setText("Index");
        ParamPanel.add(ParamIndexLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 35, -1, -1));

        ParamTypeLabel.setText("Type");
        ParamPanel.add(ParamTypeLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 60, -1, -1));

        ParamUnitLabel.setText("Unit");
        ParamPanel.add(ParamUnitLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 85, -1, -1));

        ParamPruningLabel.setText("Pruning");
        ParamPanel.add(ParamPruningLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 110, -1, -1));

        ParamValMomentLabel.setText("ValMoment");
        ParamValMomentLabel.setVerifyInputWhenFocusTarget(false);
        ParamPanel.add(ParamValMomentLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 135, -1, -1));

        ParamRTmodLabel.setText("RTmod");
        ParamPanel.add(ParamRTmodLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 160, -1, -1));

        ParamLimitsLabel.setText("Limits");
        ParamPanel.add(ParamLimitsLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 185, -1, -1));

        ParamDescriptionLabel.setText("Description");
        ParamPanel.add(ParamDescriptionLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 210, -1, -1));

        ParamNameText.setText("None");
        ParamNameText.setEnabled(false);
        ParamPanel.add(ParamNameText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 10, -1, -1));

        ParamIndexText.setText("None");
        ParamIndexText.setEnabled(false);
        ParamPanel.add(ParamIndexText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 35, -1, -1));

        ParamTypeSelection.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "node", "boolean", "int", "long", "float", "double", "icpx", "lcpx", "fcpx", "dcpx", "text", "bin", "PVSS float", "PVSS uint", "PVSS int", "PVSS float", "PVSS boolean", "PVSS text" }));
        ParamPanel.add(ParamTypeSelection, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 60, -1, -1));

        ParamUnitSelection.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "-", "Ampere", "m/s", "dB", "time4", "time6", "RAM", "GFLOP", "MB/s", "Mhz" }));
        ParamPanel.add(ParamUnitSelection, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 85, -1, -1));

        ParamPruningText.setText("None");
        ParamPanel.add(ParamPruningText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 110, -1, -1));

        ParamValMomentText.setText("None");
        ParamPanel.add(ParamValMomentText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 135, -1, -1));

        ParamRTmodText.setText("None");
        ParamPanel.add(ParamRTmodText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 160, -1, -1));

        ParamLimitsText.setText("None");
        ParamPanel.add(ParamLimitsText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 185, -1, -1));

        ParamDescriptionText.setText("None");
        ParamPanel.add(ParamDescriptionText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 210, -1, -1));

        ParamOkButton.setText("Ok");
        ParamPanel.add(ParamOkButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 250, -1, -1));

        ParamCancelButton.setText("Cancel");
        ParamPanel.add(ParamCancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 250, -1, -1));

        ParamApplyButton.setText("Apply");
        ParamPanel.add(ParamApplyButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(160, 250, -1, -1));

        treeTabbedPane.addTab("Param", null, ParamPanel, "Param Page");

        ValuePanel.setLayout(new java.awt.BorderLayout());

        jTable1.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {
                {null, null},
                {null, null},
                {null, null},
                {null, null},
                {null, null},
                {null, null},
                {null, null},
                {null, null},
                {null, null},
                {null, null},
                {null, null},
                {null, null},
                {null, null},
                {null, null},
                {null, null}
            },
            new String [] {
                "TimeStamp", "Value"
            }
        ) {
            Class[] types = new Class [] {
                java.lang.String.class, java.lang.String.class
            };
            boolean[] canEdit = new boolean [] {
                false, false
            };

            public Class getColumnClass(int columnIndex) {
                return types [columnIndex];
            }

            public boolean isCellEditable(int rowIndex, int columnIndex) {
                return canEdit [columnIndex];
            }
        });
        jTable1.setEnabled(false);
        jScrollPane1.setViewportView(jTable1);

        ValuePanel.add(jScrollPane1, java.awt.BorderLayout.CENTER);

        LogNameLabel.setText("ParamName");
        jPanel1.add(LogNameLabel);

        LogParamnameText.setText("None");
        jPanel1.add(LogParamnameText);

        ValuePanel.add(jPanel1, java.awt.BorderLayout.SOUTH);

        treeTabbedPane.addTab("Log", null, ValuePanel, "Value Page");

        BrowsePanel.add(treeTabbedPane, java.awt.BorderLayout.CENTER);

        jTabbedPane2.addTab("Browse", null, BrowsePanel, "Treeview");

        ControlPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        CtrlInstButton.setText("Inst");
        ControlPanel.add(CtrlInstButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(60, 50, -1, -1));

        CtrlDeleteButton.setText("Delete");
        ControlPanel.add(CtrlDeleteButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(60, 90, -1, -1));

        CtrlCopyButton.setText("Copy");
        ControlPanel.add(CtrlCopyButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(60, 130, -1, -1));

        CtrlChangeTreeStateButton.setText("Change Tree State");
        ControlPanel.add(CtrlChangeTreeStateButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(230, 50, -1, -1));

        jTabbedPane2.addTab("Control", null, ControlPanel, "ControlPanel");

        getContentPane().add(jTabbedPane2, java.awt.BorderLayout.CENTER);
        jTabbedPane2.getAccessibleContext().setAccessibleName("");

        SelectedLabel.setHorizontalAlignment(javax.swing.SwingConstants.RIGHT);
        SelectedLabel.setText("SelectedTree");
        infoPanel.add(SelectedLabel);

        selectedTreeTextField.setColumns(25);
        selectedTreeTextField.setEditable(false);
        selectedTreeTextField.setText("None");
        selectedTreeTextField.setOpaque(false);
        infoPanel.add(selectedTreeTextField);

        getContentPane().add(infoPanel, java.awt.BorderLayout.SOUTH);

        jMenuFile.setToolTipText("File Menu");
        jMenuFile.setLabel("File");
        jMenuFileExit.setLabel("Exit");
        jMenuFileExit.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jMenuFileExitActionPerformed(evt);
            }
        });

        jMenuFile.add(jMenuFileExit);

        jMenuBar1.add(jMenuFile);

        MenuInput.setText("Source");
        MenuInput.setToolTipText("Input selection Menu");
        jMenuInputFile.setText("File");
        jMenuInputFile.setToolTipText("Set input to file");
        jMenuInputFile.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jMenuInputFileActionPerformed(evt);
            }
        });

        MenuInput.add(jMenuInputFile);

        jMenuInputOTDB.setText("OTDB");
        jMenuInputOTDB.setToolTipText("Set input to OTDB");
        jMenuInputOTDB.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jMenuInputOTDBActionPerformed(evt);
            }
        });

        MenuInput.add(jMenuInputOTDB);

        jMenuBar1.add(MenuInput);

        setJMenuBar(jMenuBar1);

        pack();
    }
    // </editor-fold>//GEN-END:initComponents

    private void jMenuInputOTDBActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jMenuInputOTDBActionPerformed
        SettingsDialog aSW = new SettingsDialog();
        aSW.setSize(400, 300);
        aSW.setVisible(true);
    }//GEN-LAST:event_jMenuInputOTDBActionPerformed

    private void jMenuInputFileActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jMenuInputFileActionPerformed
    }//GEN-LAST:event_jMenuInputFileActionPerformed

   
    private void TreeSelectionListener(javax.swing.event.TreeSelectionEvent evt) {//GEN-FIRST:event_TreeSelectionListener
    }//GEN-LAST:event_TreeSelectionListener

    private void jMenuFileExitActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jMenuFileExitActionPerformed
        System.exit(0);
    }//GEN-LAST:event_jMenuFileExitActionPerformed
    
    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
                new otbgui().setVisible(true);
            }
        });
    }
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel BrowsePanel;
    private javax.swing.JPanel ControlPanel;
    private javax.swing.JButton CtrlChangeTreeStateButton;
    private javax.swing.JButton CtrlCopyButton;
    private javax.swing.JButton CtrlDeleteButton;
    private javax.swing.JButton CtrlInstButton;
    private javax.swing.JLabel LogNameLabel;
    private javax.swing.JTextField LogParamnameText;
    private javax.swing.JMenu MenuInput;
    private javax.swing.JButton NodeApplyButton;
    private javax.swing.JButton NodeCancelButton;
    private javax.swing.JLabel NodeDescriptionLabel;
    private javax.swing.JTextField NodeDescriptionText;
    private javax.swing.JLabel NodeIndexLabel;
    private javax.swing.JTextField NodeIndexText;
    private javax.swing.JTextField NodeInstancesText;
    private javax.swing.JLabel NodeLimitsLabel;
    private javax.swing.JTextField NodeLimitsText;
    private javax.swing.JLabel NodeNameLabel;
    private javax.swing.JTextField NodeNameText;
    private javax.swing.JButton NodeOkButton;
    private javax.swing.JPanel NodePanel;
    private javax.swing.JButton ParamApplyButton;
    private javax.swing.JButton ParamCancelButton;
    private javax.swing.JLabel ParamDescriptionLabel;
    private javax.swing.JTextField ParamDescriptionText;
    private javax.swing.JLabel ParamIndexLabel;
    private javax.swing.JTextField ParamIndexText;
    private javax.swing.JLabel ParamLimitsLabel;
    private javax.swing.JTextField ParamLimitsText;
    private javax.swing.JLabel ParamNameLabel;
    private javax.swing.JTextField ParamNameText;
    private javax.swing.JButton ParamOkButton;
    private javax.swing.JPanel ParamPanel;
    private javax.swing.JLabel ParamPruningLabel;
    private javax.swing.JTextField ParamPruningText;
    private javax.swing.JLabel ParamRTmodLabel;
    private javax.swing.JTextField ParamRTmodText;
    private javax.swing.JLabel ParamTypeLabel;
    private javax.swing.JComboBox ParamTypeSelection;
    private javax.swing.JLabel ParamUnitLabel;
    private javax.swing.JComboBox ParamUnitSelection;
    private javax.swing.JLabel ParamValMomentLabel;
    private javax.swing.JTextField ParamValMomentText;
    private javax.swing.JLabel SelectedLabel;
    private javax.swing.JButton TreeApplyButton;
    private javax.swing.JPanel TreePanel;
    private javax.swing.JTable TreeTable;
    private javax.swing.JPanel ValuePanel;
    private javax.swing.JTree departmentTree;
    private javax.swing.JPanel infoPanel;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JMenuBar jMenuBar1;
    private javax.swing.JMenu jMenuFile;
    private javax.swing.JMenuItem jMenuFileExit;
    private javax.swing.JMenuItem jMenuInputFile;
    private javax.swing.JMenuItem jMenuInputOTDB;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JTabbedPane jTabbedPane2;
    private javax.swing.JTable jTable1;
    private javax.swing.JTextField selectedTreeTextField;
    private javax.swing.JScrollPane treeScrollPane;
    private javax.swing.JTabbedPane treeTabbedPane;
    // End of variables declaration//GEN-END:variables
    
    private boolean fillTree() {
        boolean status = false;
        boolean skip = false;
        String lastName="";
        
        String aTreeLevelName="";
        status = true;
        /** 
         *fill the treeview. In this case from the array, but later it can be a file or database
         *
         */
       
        for (Iterator<String[]> iter = people.iterator(); iter.hasNext();) {                      
           // data contains every single new line from the inputfile
           data=iter.next();
           // treeLine contains the usenet Treelist
           treeLine = data[0].split("[.]");
           if (aTreeLevelName == "") {
              root.setUserObject(new DefaultMutableTreeNode(treeLine[0]));
              aTreeLevelName=treeLine[0];
           }
           
           
           aNode = root;
           DefaultMutableTreeNode oldNode = aNode;              

           if (! treeLine[treeLine.length-1].equals(lastName)) {
              for (int i=1; i < treeLine.length;i++) {
                 if (!findChild(aNode, treeLine[i])){
                    aNode = new DefaultMutableTreeNode(treeLine[i]);
                    oldNode.add(aNode);
                 }
                 oldNode=aNode;
              }
              lastName=treeLine[treeLine.length-1];
           }
        }
        departmentTree.updateUI();
        return status;
    }
    
    boolean findChild(DefaultMutableTreeNode aRoot, String aName){
        Enumeration e = aRoot.children();
        DefaultMutableTreeNode aNewNode;
        while (e.hasMoreElements()){
           aNewNode= (DefaultMutableTreeNode)e.nextElement();
           if (aNewNode.getUserObject().toString().equals(aName)) {
               aNode=aNewNode;
               return true;
           }
        }
        return false;
    }
    




 
   
    private boolean readFile(File aFile){
        people.clear();
        boolean status=true;
        try {
            BufferedReader in = new BufferedReader(new FileReader(aFile));
            String aStr;
            while ((aStr = in.readLine()) != null) {
                status=processLine(aStr);
            }
            in.close();        
        } catch (IOException e) {
            System.out.println("Error reading from file: "+aFile.getName());
            e.printStackTrace();
            status=false;
        }
        return status;
    }

    private boolean processLine(String aStr){
     boolean status = true;
     // find the last aaa="bbb" combination
     String [] aS1 = aStr.split("[=]");
     String [] aS2 = aS1[0].split("[.]");
     
     String [] saveString= new String[2];
     for (int i=0; i< aS2.length-1;i++) {
         if (i>0) {
             saveString[0]+="."+aS2[i];
         } else {
             saveString[0]=aS2[i];
         }
     }
     saveString[1]=aS2[aS2.length-1]+"="+aS1[1];
     saveString[0]=saveString[0].replace('_',' ');
     saveString[1]=saveString[1].replace('_',' ');
     people.add(saveString);
     
     
     return status;
    }
}
