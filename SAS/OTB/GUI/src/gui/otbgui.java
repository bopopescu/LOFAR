/*

 * otbgui.java

 *

 * Created on April 26, 2005, 2:09 PM

 */



package gui;



import jOTDB.jOTDBinterface;
import java.io.File;
import java.rmi.Naming;
import java.util.Vector;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.ListSelectionModel;
import javax.swing.tree.DefaultMutableTreeNode;









/**
 *
 * @author  Alex Gerdes & Arthur Coolen
 */





public class otbgui extends javax.swing.JFrame {

    
     // SettingsDialog Defaults
    private static String RMIServerName   = "lofar17.astron.nl";
    private static String RMIServerPort   = "1099";
    private static String RMIRegistryName = "jOTDB";
    private static String OTDBUserName    = "paulus";
    private static String OTDBPassword    = "boskabouter";
    private static String OTDBDBName      = "otdbtest";   
    
    // Param Defaults
    private String aParamName        = "None";
    private String aParamIndex       = "-1";
    private int aParamType           = 0;
    private int aParamUnit           = 0;
    private String aParamPruning     = "-1";
    private String aParamValMoment   = "-1";
    private int aParamRTMod          = 0;
    private String aParamLimits      = "None";
    private String aParamDescription = "None";
    
    // Node defaults
    private String aNodeName         = "None";
    private String aNodeIndex        = "-1";
    private int aNodeLeaf            = 0;
    private String aNodeInstances    = "-1";
    private String aNodeLimits       = "None";
    private String aNodeDescription  = "None";
    
    
    private static jOTDBinterface remoteOTDB;    
    
    private static String aSelectedTree   = "None";
    
    
    DefaultMutableTreeNode root = new DefaultMutableTreeNode();
    

    
    private boolean isTreeFilled = false;
    private boolean isOTDB =false;
    private boolean isFile=true;
       
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
        TreeTableScrollPane = new javax.swing.JScrollPane();
        TreeTable = new javax.swing.JTable();
        TreeSelectButton = new javax.swing.JButton();
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
        ParamLimitsText = new javax.swing.JTextField();
        ParamDescriptionText = new javax.swing.JTextField();
        ParamOkButton = new javax.swing.JButton();
        ParamCancelButton = new javax.swing.JButton();
        ParamApplyButton = new javax.swing.JButton();
        ParamRTModSelection = new javax.swing.JComboBox();
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
        FileMenu = new javax.swing.JMenu();
        FileMenuExit = new javax.swing.JMenuItem();
        SourceMenu = new javax.swing.JMenu();
        SourceMenuInputFile = new javax.swing.JRadioButtonMenuItem();
        SourceMenuInputOTDB = new javax.swing.JRadioButtonMenuItem();
        SettingsMenu = new javax.swing.JMenu();
        SettingsMenuRMISettings = new javax.swing.JMenuItem();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setMaximizedBounds(new java.awt.Rectangle(0, 0, 1000, 400));
        setName("mainFrame");
        jTabbedPane2.setMinimumSize(new java.awt.Dimension(700, 300));
        jTabbedPane2.setPreferredSize(new java.awt.Dimension(700, 300));
        TreePanel.setLayout(new java.awt.BorderLayout());

        TreePanel.setToolTipText("Search on one or more of the given constraints");
        TreePanel.setMinimumSize(new java.awt.Dimension(700, 400));
        TreePanel.setPreferredSize(new java.awt.Dimension(700, 400));
        TreeTable.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {

            },
            new String [] {
                "TreeID", "Creator", "Creation Date", "Class", "Type", "State", "Campaign", "Start Time", "Stop Time"
            }
        ) {
            Class[] types = new Class [] {
                java.lang.String.class, java.lang.String.class, java.lang.String.class, java.lang.String.class, java.lang.String.class, java.lang.String.class, java.lang.String.class, java.lang.String.class, java.lang.String.class
            };
            boolean[] canEdit = new boolean [] {
                false, false, false, false, false, false, false, false, false
            };

            public Class getColumnClass(int columnIndex) {
                return types [columnIndex];
            }

            public boolean isCellEditable(int rowIndex, int columnIndex) {
                return canEdit [columnIndex];
            }
        });
        TreeTableScrollPane.add(TreeTable);
        TreeTableScrollPane.setViewportView(TreeTable);

        TreePanel.add(TreeTableScrollPane, java.awt.BorderLayout.CENTER);

        TreeSelectButton.setText("Select");
        TreeSelectButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                TreeSelectButtonActionPerformed(evt);
            }
        });

        TreePanel.add(TreeSelectButton, java.awt.BorderLayout.SOUTH);

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
        NodeOkButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                NodeOkButtonActionPerformed(evt);
            }
        });

        NodePanel.add(NodeOkButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 250, -1, -1));

        NodeCancelButton.setText("Cancel");
        NodeCancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                NodeCancelButtonActionPerformed(evt);
            }
        });

        NodePanel.add(NodeCancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 250, -1, -1));

        NodeApplyButton.setText("Apply");
        NodeApplyButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                NodeApplyButtonActionPerformed(evt);
            }
        });

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

        ParamLimitsText.setText("None");
        ParamPanel.add(ParamLimitsText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 185, -1, -1));

        ParamDescriptionText.setText("None");
        ParamPanel.add(ParamDescriptionText, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 210, -1, -1));

        ParamOkButton.setText("Ok");
        ParamOkButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ParamOkButtonActionPerformed(evt);
            }
        });

        ParamPanel.add(ParamOkButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(40, 250, -1, -1));

        ParamCancelButton.setText("Cancel");
        ParamCancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ParamCancelButtonActionPerformed(evt);
            }
        });

        ParamPanel.add(ParamCancelButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(90, 250, -1, -1));

        ParamApplyButton.setText("Apply");
        ParamApplyButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                ParamApplyButtonActionPerformed(evt);
            }
        });

        ParamPanel.add(ParamApplyButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(160, 250, -1, -1));

        ParamRTModSelection.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "False", "True" }));
        ParamPanel.add(ParamRTModSelection, new org.netbeans.lib.awtextra.AbsoluteConstraints(140, 160, -1, -1));

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

        ControlPanel.setPreferredSize(new java.awt.Dimension(700, 450));
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

        FileMenu.setToolTipText("File Menu");
        FileMenu.setLabel("File");
        FileMenuExit.setLabel("Exit");
        FileMenuExit.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                FileMenuExitActionPerformed(evt);
            }
        });

        FileMenu.add(FileMenuExit);

        jMenuBar1.add(FileMenu);

        SourceMenu.setText("Source");
        SourceMenu.setToolTipText("Source selection Menu");
        SourceMenuInputFile.setSelected(true);
        SourceMenuInputFile.setText("Input from File");
        SourceMenuInputFile.setToolTipText("Set input to file");
        SourceMenuInputFile.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SourceMenuInputFileActionPerformed(evt);
            }
        });

        SourceMenu.add(SourceMenuInputFile);

        SourceMenuInputOTDB.setText("Input from OTDB");
        SourceMenuInputOTDB.setToolTipText("Set input to OTDB");
        SourceMenuInputOTDB.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SourceMenuInputOTDBActionPerformed(evt);
            }
        });

        SourceMenu.add(SourceMenuInputOTDB);

        jMenuBar1.add(SourceMenu);

        SettingsMenu.setText("Settings");
        SettingsMenu.setToolTipText("Settings Menu");
        SettingsMenuRMISettings.setText("RMISettings");
        SettingsMenuRMISettings.setToolTipText("set RMI server & database");
        SettingsMenuRMISettings.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                SettingsMenuRMISettingsActionPerformed(evt);
            }
        });

        SettingsMenu.add(SettingsMenuRMISettings);

        jMenuBar1.add(SettingsMenu);

        setJMenuBar(jMenuBar1);

        pack();
    }
    // </editor-fold>//GEN-END:initComponents

    private void ParamCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParamCancelButtonActionPerformed
        // ParamNameText.setText(aParamName);
        // ParamIndexText.setText(aParamIndex);
        ParamTypeSelection.setSelectedIndex(aParamType);
        ParamUnitSelection.setSelectedIndex(aParamUnit);
        ParamPruningText.setText(aParamPruning);
        ParamValMomentText.setText(aParamValMoment);
        ParamRTModSelection.setSelectedIndex(aParamRTMod);
        ParamLimitsText.setText(aParamLimits);
        ParamDescriptionText.setText(aParamDescription);
    }//GEN-LAST:event_ParamCancelButtonActionPerformed

    private void setParamValues() {
        // aParamName=ParamNameText.getText();
        // aParamIndex=ParamIndexText.getText();
        aParamType=(short)ParamTypeSelection.getSelectedIndex();
        aParamUnit=(short)ParamUnitSelection.getSelectedIndex();
        aParamPruning=ParamPruningText.getText();
        aParamValMoment=ParamValMomentText.getText();
        aParamRTMod=ParamRTModSelection.getSelectedIndex();
        aParamLimits=ParamLimitsText.getText();
        aParamDescription=ParamDescriptionText.getText();
    }
    
    private void ParamOkButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParamOkButtonActionPerformed
        setParamValues();
    }//GEN-LAST:event_ParamOkButtonActionPerformed

    private void setNodeValues() {
        // aNodeName=NodeNameText.getText();
        // aNodeIndex=NodeIndexText.getText();
        aNodeInstances=NodeInstancesText.getText();
        aNodeLimits=NodeLimitsText.getText();
        aNodeDescription=NodeDescriptionText.getText();
    }
    
    private void NodeCancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NodeCancelButtonActionPerformed
        // NodeNameText.setText(aNodeName);
        // NodeIndexText.setText(aNodeIndex);
        NodeInstancesText.setText(aNodeInstances);
        NodeLimitsText.setText(aNodeLimits);
        NodeDescriptionText.setText(aNodeDescription);
    }//GEN-LAST:event_NodeCancelButtonActionPerformed

    private void NodeOkButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NodeOkButtonActionPerformed
        setNodeValues();
    }//GEN-LAST:event_NodeOkButtonActionPerformed

    private void ParamApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_ParamApplyButtonActionPerformed
        setParamValues();
    }//GEN-LAST:event_ParamApplyButtonActionPerformed

    private void NodeApplyButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_NodeApplyButtonActionPerformed
        setNodeValues();
    }//GEN-LAST:event_NodeApplyButtonActionPerformed

    private void TreeSelectButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_TreeSelectButtonActionPerformed
        aSelectedTree = "None";
        int aRow= TreeTable.getSelectedRow();
        if ( aRow != -1) {
            aSelectedTree=TreeTable.getValueAt(aRow, 0).toString()+"."+
                    TreeTable.getValueAt(aRow, 3).toString()+"."+
                    TreeTable.getValueAt(aRow, 4).toString()+"."+
                    TreeTable.getValueAt(aRow, 5).toString();
        }
        selectedTreeTextField.setText(aSelectedTree);
        
        // Start filling the TreeView
        if (aSelectedTree != "None") {
            //getTreeList();
        }
        
    }//GEN-LAST:event_TreeSelectButtonActionPerformed


    private void SourceMenuInputOTDBActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SourceMenuInputOTDBActionPerformed
        aSelectedTree = "None";
        selectedTreeTextField.setText(aSelectedTree);

        // set default failed values
        SourceMenuInputFile.setSelected(false);
        SourceMenuInputFile.setSelected(false);
        isFile=false;
        isOTDB=false;

        if (RMIServerName.length()==0 || RMIServerPort.length()==0 ||
              RMIRegistryName.length()==0 || OTDBUserName.length() ==0 ||
              OTDBPassword.length()==0 || OTDBDBName.length()==0) {

            JOptionPane.showMessageDialog(null,"You didn't set the RMI settings",
                    "Source selection warning",
                    JOptionPane.WARNING_MESSAGE);
        }  else {
          try {
            String aS="rmi://"+RMIServerName+":"+RMIServerPort+"/"+RMIRegistryName;
//            String aS="lofar17.astron.nl";
            if (openRemoteConnection(aS)) {
               if (fillTreeTable()) {
                  System.out.println("Table should be filled now");
                  SourceMenuInputOTDB.setSelected(true);
                  isOTDB=true;
               } else {
                  System.out.println("Error in filling the table");
               }
             }          } 
          catch (Exception e)
          {
             System.out.println ("Remote OTDB via RMI and JNI failed: " + e); 
          }
      }

    }//GEN-LAST:event_SourceMenuInputOTDBActionPerformed



    private void SourceMenuInputFileActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SourceMenuInputFileActionPerformed
      aSelectedTree = "None";
      selectedTreeTextField.setText(aSelectedTree);
      JFileChooser fc = new JFileChooser();
      int returnVal = fc.showOpenDialog(otbgui.this);
      if (returnVal == JFileChooser.APPROVE_OPTION) {
          File aFile = fc.getSelectedFile();
          SourceMenuInputOTDB.setSelected(true);
          isFile=true;
          isOTDB=false;
      } else {
          JOptionPane.showMessageDialog(null,"You didn't select a file",
                  "Source selection warning",
                  JOptionPane.WARNING_MESSAGE);
          SourceMenuInputFile.setSelected(false);
      }
    }//GEN-LAST:event_SourceMenuInputFileActionPerformed



    private void SettingsMenuRMISettingsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_SettingsMenuRMISettingsActionPerformed
        SettingsDialog aSW = new SettingsDialog();
        aSW.setSize(400, 300);
        aSW.setVisible(true);     
    }//GEN-LAST:event_SettingsMenuRMISettingsActionPerformed



   

    private void TreeSelectionListener(javax.swing.event.TreeSelectionEvent evt) {//GEN-FIRST:event_TreeSelectionListener

    }//GEN-LAST:event_TreeSelectionListener



    private void FileMenuExitActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_FileMenuExitActionPerformed

        System.exit(0);
    }//GEN-LAST:event_FileMenuExitActionPerformed

    

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
    private javax.swing.JMenu FileMenu;
    private javax.swing.JMenuItem FileMenuExit;
    private javax.swing.JLabel LogNameLabel;
    private javax.swing.JTextField LogParamnameText;
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
    private javax.swing.JComboBox ParamRTModSelection;
    private javax.swing.JLabel ParamRTmodLabel;
    private javax.swing.JLabel ParamTypeLabel;
    private javax.swing.JComboBox ParamTypeSelection;
    private javax.swing.JLabel ParamUnitLabel;
    private javax.swing.JComboBox ParamUnitSelection;
    private javax.swing.JLabel ParamValMomentLabel;
    private javax.swing.JTextField ParamValMomentText;
    private javax.swing.JLabel SelectedLabel;
    private javax.swing.JMenu SettingsMenu;
    private javax.swing.JMenuItem SettingsMenuRMISettings;
    private javax.swing.JMenu SourceMenu;
    private javax.swing.JRadioButtonMenuItem SourceMenuInputFile;
    private javax.swing.JRadioButtonMenuItem SourceMenuInputOTDB;
    private javax.swing.JPanel TreePanel;
    private javax.swing.JButton TreeSelectButton;
    private javax.swing.JTable TreeTable;
    private javax.swing.JScrollPane TreeTableScrollPane;
    private javax.swing.JPanel ValuePanel;
    private javax.swing.JTree departmentTree;
    private javax.swing.JPanel infoPanel;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JMenuBar jMenuBar1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JTabbedPane jTabbedPane2;
    private javax.swing.JTable jTable1;
    private javax.swing.JTextField selectedTreeTextField;
    private javax.swing.JScrollPane treeScrollPane;
    private javax.swing.JTabbedPane treeTabbedPane;
    // End of variables declaration//GEN-END:variables

    private boolean openRemoteConnection(String RMIRegHostName) {
        try {
            System.out.println("Starting... for "+RMIRegHostName);
        
            // create a remote object
            remoteOTDB = (jOTDBinterface) Naming.lookup (RMIRegHostName);     
//	    Registry remoteRegistry = LocateRegistry.getRegistry(RMIRegHostName.toCharArray()[0]);
//	    remoteOTDB = (jOTDBinterface) remoteRegistry.lookup (jOTDBinterface.SERVICENAME);
            System.out.println (remoteOTDB);
					    
	    // do the test	
	    System.out.println("Trying to connect to the database");
	    assert remoteOTDB.connect() : "Connection failed";	
	    assert remoteOTDB.isConnected() : "Connnection flag failed";
	     
	    System.out.println("Connection succesful!");   
            return true;
          }
        catch (Exception e)
	  {
	     System.out.println ("Remote OTDB via RMI and JNI failed: " + e);
	  }
        return false;
    }   
    
    private boolean fillTreeTable() {
        System.out.println("Trying to obtain the treeList");
        Vector treeList=new Vector();
        
       
        try {            
          treeList=remoteOTDB.getTreeList((short)0, (short)0);
          if (treeList.size() == 0) {
            System.out.println("Error:" + remoteOTDB.errorMsg());
            return false;
          } else {
            System.out.println("Collected tree list");
          }
        } 
        catch (Exception e)
	{
	  System.out.println ("Remote OTDB via RMI and JNI failed: " + e);
	}        System.out.println("Trying to obtain the treeList");

        // Give our own model to the JFrame Object
        treeModel tm = new treeModel(remoteOTDB,treeList);
        TreeTable.setModel(tm);
        TreeTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        return true;
    }
            
}    

 
            
            

