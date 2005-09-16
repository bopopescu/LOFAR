/*
 * otbgui.java
 *
 * Created on April 26, 2005, 2:09 PM
 */

package otbtest;

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
        javax.swing.JLabel privatePhoneLabel;

        jTabbedPane2 = new javax.swing.JTabbedPane();
        TreePanel = new javax.swing.JPanel();
        treeScrollPane = new javax.swing.JScrollPane();
        departmentTree = new javax.swing.JTree(root);
        treeTabbedPane = new javax.swing.JTabbedPane();
        SalarisPanel = new javax.swing.JPanel();
        salaryLabel = new javax.swing.JLabel();
        salaryTextField = new javax.swing.JTextField();
        AddressPanel = new javax.swing.JPanel();
        nameLabel = new javax.swing.JLabel();
        nameTextField = new javax.swing.JTextField();
        addressLabel = new javax.swing.JLabel();
        addressTextField = new javax.swing.JTextField();
        postalCodeLabel = new javax.swing.JLabel();
        postalCodeTextField = new javax.swing.JTextField();
        cityLabel = new javax.swing.JLabel();
        cityTextField = new javax.swing.JTextField();
        phoneLabel = new javax.swing.JLabel();
        phoneTextField = new javax.swing.JTextField();
        privatePhoneLabel = new javax.swing.JLabel();
        privatePhoneTextField = new javax.swing.JTextField();
        emailLabel = new javax.swing.JLabel();
        emailTextField = new javax.swing.JTextField();
        QueryPanel = new javax.swing.JPanel();
        searchNameLabel = new javax.swing.JLabel();
        searchNameTextField = new javax.swing.JTextField();
        searchAddressLabel = new javax.swing.JLabel();
        searchAddressTextField = new javax.swing.JTextField();
        serachCityLabel = new javax.swing.JLabel();
        searchCityTextField = new javax.swing.JTextField();
        search = new javax.swing.JButton();
        selectButton = new javax.swing.JButton();
        clearButton = new javax.swing.JButton();
        jScrollPane2 = new javax.swing.JScrollPane();
        resultList = new javax.swing.JList();
        infoPanel = new javax.swing.JPanel();
        SelectedLabel = new javax.swing.JLabel();
        selectedTextField = new javax.swing.JTextField();
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
        TreePanel.setLayout(new java.awt.BorderLayout());

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

        TreePanel.add(treeScrollPane, java.awt.BorderLayout.WEST);

        salaryLabel.setText("Salary");
        salaryLabel.setRequestFocusEnabled(false);
        SalarisPanel.add(salaryLabel);
        salaryLabel.getAccessibleContext().setAccessibleName("SalarisLabel");

        salaryTextField.setEditable(false);
        salaryTextField.setText("0");
        salaryTextField.setMinimumSize(new java.awt.Dimension(100, 19));
        salaryTextField.setPreferredSize(new java.awt.Dimension(100, 19));
        SalarisPanel.add(salaryTextField);

        treeTabbedPane.addTab("Salary", null, SalarisPanel, "Show Salary");

        AddressPanel.setLayout(new java.awt.GridLayout(7, 2));

        nameLabel.setText("Name");
        AddressPanel.add(nameLabel);

        nameTextField.setColumns(50);
        nameTextField.setEditable(false);
        nameTextField.setMinimumSize(new java.awt.Dimension(100, 19));
        AddressPanel.add(nameTextField);

        addressLabel.setText("Address");
        AddressPanel.add(addressLabel);

        addressTextField.setColumns(50);
        addressTextField.setEditable(false);
        AddressPanel.add(addressTextField);

        postalCodeLabel.setText("Postal Code");
        AddressPanel.add(postalCodeLabel);

        postalCodeTextField.setColumns(50);
        postalCodeTextField.setEditable(false);
        AddressPanel.add(postalCodeTextField);

        cityLabel.setText("City");
        AddressPanel.add(cityLabel);

        cityTextField.setColumns(50);
        cityTextField.setEditable(false);
        AddressPanel.add(cityTextField);

        phoneLabel.setText("Phone");
        AddressPanel.add(phoneLabel);

        phoneTextField.setColumns(50);
        phoneTextField.setEditable(false);
        AddressPanel.add(phoneTextField);

        privatePhoneLabel.setText("Private Phone");
        AddressPanel.add(privatePhoneLabel);

        privatePhoneTextField.setColumns(50);
        privatePhoneTextField.setEditable(false);
        AddressPanel.add(privatePhoneTextField);

        emailLabel.setText("email");
        emailLabel.setAutoscrolls(true);
        AddressPanel.add(emailLabel);

        emailTextField.setColumns(50);
        emailTextField.setEditable(false);
        AddressPanel.add(emailTextField);

        treeTabbedPane.addTab("Address", null, AddressPanel, "toon adresgegevens");

        TreePanel.add(treeTabbedPane, java.awt.BorderLayout.CENTER);

        jTabbedPane2.addTab("Tree", null, TreePanel, "Treeview");

        QueryPanel.setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        QueryPanel.setToolTipText("Search on one or more of the given constraints");
        QueryPanel.setMinimumSize(new java.awt.Dimension(500, 300));
        QueryPanel.setPreferredSize(new java.awt.Dimension(500, 300));
        searchNameLabel.setText("Name");
        QueryPanel.add(searchNameLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(60, 100, 80, 20));

        searchNameTextField.setColumns(50);
        QueryPanel.add(searchNameTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 100, 220, -1));

        searchAddressLabel.setText("Address");
        QueryPanel.add(searchAddressLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(60, 120, 110, 20));

        searchAddressTextField.setColumns(50);
        QueryPanel.add(searchAddressTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 120, 220, -1));

        serachCityLabel.setText("City");
        QueryPanel.add(serachCityLabel, new org.netbeans.lib.awtextra.AbsoluteConstraints(60, 140, 100, 20));

        searchCityTextField.setColumns(50);
        QueryPanel.add(searchCityTextField, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 140, 220, -1));

        search.setText("Search");
        search.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                searchActionPerformed(evt);
            }
        });

        QueryPanel.add(search, new org.netbeans.lib.awtextra.AbsoluteConstraints(150, 200, 90, -1));

        selectButton.setText("Select");
        selectButton.setToolTipText("place the given selection in the different fields");
        selectButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                selectButtonActionPerformed(evt);
            }
        });

        QueryPanel.add(selectButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(500, 240, -1, -1));

        clearButton.setText("Clear");
        clearButton.setToolTipText("Celar the selection fields");
        clearButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                clearButtonActionPerformed(evt);
            }
        });

        QueryPanel.add(clearButton, new org.netbeans.lib.awtextra.AbsoluteConstraints(250, 200, 90, -1));

        resultList.setBorder(new javax.swing.border.EtchedBorder());
        resultList.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        resultList.setMaximumSize(new java.awt.Dimension(150, 200));
        resultList.setMinimumSize(new java.awt.Dimension(150, 200));
        resultList.setPreferredSize(new java.awt.Dimension(200, 400));
        jScrollPane2.setViewportView(resultList);

        QueryPanel.add(jScrollPane2, new org.netbeans.lib.awtextra.AbsoluteConstraints(380, 30, 310, 200));

        jTabbedPane2.addTab("Query", null, QueryPanel, "change tree search");

        getContentPane().add(jTabbedPane2, java.awt.BorderLayout.NORTH);
        jTabbedPane2.getAccessibleContext().setAccessibleName("");

        SelectedLabel.setHorizontalAlignment(javax.swing.SwingConstants.RIGHT);
        SelectedLabel.setText("Selected");
        infoPanel.add(SelectedLabel);

        selectedTextField.setColumns(25);
        selectedTextField.setEditable(false);
        selectedTextField.setText("None");
        selectedTextField.setOpaque(false);
        infoPanel.add(selectedTextField);

        getContentPane().add(infoPanel, java.awt.BorderLayout.CENTER);

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

        MenuInput.setText("Input");
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
        jMenuInputOTDB.setToolTipText("Set input to file");
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
        clearAll();
        if (getServerList()) {
           isTreeFilled=fillTree();
       } else {
           System.out.println("Error getting RMI Access, tree not filled");
       }

    }//GEN-LAST:event_jMenuInputOTDBActionPerformed

    public static boolean getServerList() {
        try {
//            navigatorRMI aRMIInterface = (navigatorRMI) Naming.lookup ("rmi://coole372.speed.planet.nl:10099/navigatorRMItje");
//            navigatorRMI aRMIInterface = (navigatorRMI) Naming.lookup ("rmi://10.0.0.154:10099/navigatorRMItje");
            navigatorRMI aRMIInterface = (navigatorRMI) Naming.lookup ("rmi://dop32.astron.nl:10099/navigatorRMItje");
            people = aRMIInterface.getList();
            System.out.println("People should be filled now");
            return true;
        } catch (Exception e) {
            System.out.println ("navigatorRMI exception: " + e);
            return false;
        }        
    }
    private void jMenuInputFileActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jMenuInputFileActionPerformed
        // clear all fields
        clearAll();
        JFileChooser fc = new JFileChooser();
        int returnVal = fc.showOpenDialog(otbgui.this);
        if (returnVal == JFileChooser.APPROVE_OPTION) {
            File aFile = fc.getSelectedFile();
            readFile(aFile);
            isTreeFilled=fillTree();
        }         
    }//GEN-LAST:event_jMenuInputFileActionPerformed

    private void clearAll() {
       // empty people
       if (people != null) {
          people.clear();
       }
       people = new ArrayList<String []>();
       
       root.removeAllChildren();
       
       clearSalaryForm();
       clearAddressForm();
       clearSearchForm();
       clearInfoForm();
       
       isTreeFilled=false;
       departmentTree.updateUI();
    }
    
    private void clearSalaryForm() {
       salaryTextField.setText("");
       SalarisPanel.updateUI();
    }
    
    private void clearAddressForm() {
       nameTextField.setText("");
       addressTextField.setText("");
       postalCodeTextField.setText("");
       cityTextField.setText("");
       phoneTextField.setText("");
       privatePhoneTextField.setText("");
       emailTextField.setText("");
       AddressPanel.updateUI();
    }
    
    private void clearSearchForm() {
        searchNameTextField.setText("");
        searchAddressTextField.setText("");
        searchCityTextField.setText("");
        resultList.removeAll();
        String aS[]={};
        resultList.setListData(aS);
        QueryPanel.updateUI();
    }
    
    private void clearInfoForm() {
        selectedTextField.setText("");
        infoPanel.updateUI();
    }
    private void clearButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_clearButtonActionPerformed
        clearSearchForm();
    }//GEN-LAST:event_clearButtonActionPerformed

    private void searchActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_searchActionPerformed
         /* Look if fields are filled to search the database for */
        String aName     = searchNameTextField.getText();
        String anAddress = searchAddressTextField.getText();
        String aCity     = searchCityTextField.getText();
        String oldPerson = "";
        Boolean found    = false;
        Boolean contSearch = true;
        List aList = new List(10,false);
        
        if (!isTreeFilled) {
            System.out.println("First fill the treeview");
            return;
        }
        
        for (Iterator<String[]> iter = people.iterator(); iter.hasNext();) {

            String [] data=iter.next();
            String [] aS=data[0].split("[.]");
            String aSearchType=data[1].split("[=]")[0];
            String aSearchString=data[1].split("[=]")[1];
                    
            String aPerson=aS[aS.length-1];
            
            // if person changes reset all flags
            if (!oldPerson.equals(aPerson)) {
                if (found && contSearch) {
                   aList.addItem(oldPerson);                    
                }
                oldPerson=aPerson;
                found = false;
                contSearch=true;
            }
            
            // check if person matches are needed and if this person qualifies
            if (aName.length() > 0  && contSearch) {
                if (aPerson.contains(aName)) {
                    found=true;
                } else {
                    found = false;
                    contSearch = false;
                }
            }
            
            // check if address matches are needed and if this address qualifies
            if (anAddress.length() > 0  && contSearch) {
                if (aSearchType.equals("Address")){
                   if (aSearchString.contains(anAddress)) {
                      found=true;
                   } else {
                      found = false;
                      contSearch = false;
                   }
                }
            }

            // check if city matches are needed and if this city qualifies
            if (aCity.length() > 0  && contSearch) {
                if (aSearchType.equals("City")) {
                    if (aSearchString.contains(aCity)) {
                       found=true;
                    } else {
                       found = false;
                       contSearch = false;
                    }
                }
            }
        }
        resultList.setListData(aList.getItems());
    }//GEN-LAST:event_searchActionPerformed

    
    private void selectButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_selectButtonActionPerformed
        if (resultList.getSelectedIndex() > -1) {
            changeSelection(resultList.getSelectedValue().toString());
              // TODO 
        }
    }//GEN-LAST:event_selectButtonActionPerformed

    private void TreeSelectionListener(javax.swing.event.TreeSelectionEvent evt) {//GEN-FIRST:event_TreeSelectionListener
        TreePath tp = evt.getNewLeadSelectionPath();
        if (tp != null) {
            changeTreeSelection(tp.getLastPathComponent());
        }
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
    private javax.swing.JPanel AddressPanel;
    private javax.swing.JMenu MenuInput;
    private javax.swing.JPanel QueryPanel;
    private javax.swing.JPanel SalarisPanel;
    private javax.swing.JLabel SelectedLabel;
    private javax.swing.JPanel TreePanel;
    private javax.swing.JLabel addressLabel;
    private javax.swing.JTextField addressTextField;
    private javax.swing.JLabel cityLabel;
    private javax.swing.JTextField cityTextField;
    private javax.swing.JButton clearButton;
    private javax.swing.JTree departmentTree;
    private javax.swing.JLabel emailLabel;
    private javax.swing.JTextField emailTextField;
    private javax.swing.JPanel infoPanel;
    private javax.swing.JMenuBar jMenuBar1;
    private javax.swing.JMenu jMenuFile;
    private javax.swing.JMenuItem jMenuFileExit;
    private javax.swing.JMenuItem jMenuInputFile;
    private javax.swing.JMenuItem jMenuInputOTDB;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JTabbedPane jTabbedPane2;
    private javax.swing.JLabel nameLabel;
    private javax.swing.JTextField nameTextField;
    private javax.swing.JLabel phoneLabel;
    private javax.swing.JTextField phoneTextField;
    private javax.swing.JLabel postalCodeLabel;
    private javax.swing.JTextField postalCodeTextField;
    private javax.swing.JTextField privatePhoneTextField;
    private javax.swing.JList resultList;
    private javax.swing.JLabel salaryLabel;
    private javax.swing.JTextField salaryTextField;
    private javax.swing.JButton search;
    private javax.swing.JLabel searchAddressLabel;
    private javax.swing.JTextField searchAddressTextField;
    private javax.swing.JTextField searchCityTextField;
    private javax.swing.JLabel searchNameLabel;
    private javax.swing.JTextField searchNameTextField;
    private javax.swing.JButton selectButton;
    private javax.swing.JTextField selectedTextField;
    private javax.swing.JLabel serachCityLabel;
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
    
    private void changeTreeSelection(Object aO) {
        String aS = aO.toString();
        changeSelection(aS);
    }
    
    private void changeSelection(String aPerson){
        boolean found = false;
        
        // No reason to do this when Tree is not filled yet anyway
        if (! isTreeFilled) return;
        
        //find arrayindex that contains this person
        Enumeration e = ((DefaultMutableTreeNode)departmentTree.getModel().getRoot()).breadthFirstEnumeration();
        
        DefaultMutableTreeNode aFoundNode;
        boolean personDone=false;
        while (e.hasMoreElements()) {
            aFoundNode=(DefaultMutableTreeNode)e.nextElement();
            if (aFoundNode.getUserObject().toString().equals(aPerson) && aFoundNode.isLeaf()) {
                for (Iterator<String []>  iter = people.iterator(); iter.hasNext();) {
                    String [] data=iter.next();
                    String [] aS=data[0].split("[.]");
                    if (aPerson.equals(aS[aS.length-1])){
                        if (!personDone){
                            personDone=true;
                            changePerson(aPerson);
                        }
                        String [] aTarget=data[1].split("[=]");
                        changeTextField(aTarget);
                    }
                 }
                 selectedTextField.setText(aPerson);
                 selectedTextField.setColumns(aPerson.length());
                 selectedTextField.updateUI();
                 
                 found = true;
                 TreePath aTreePath = new TreePath(aFoundNode.getPath());
                 departmentTree.scrollPathToVisible(aTreePath);
                 departmentTree.expandPath(aTreePath);
                 departmentTree.setSelectionPath(aTreePath);
            }
        }
    }

    private void changeTextField(String [] aS) {
        aS[1]=aS[1].replaceAll("\"","");
        if (aS[0].equals("Salary")) {
            changeSalary(aS[1]);
        } else if (aS[0].equals("Address")) {
            changeAddress(aS[1]);
        } else if (aS[0].equals("PostalCode")) {
            changePostalCode(aS[1]);
        } else if (aS[0].equals("City")) {
            changeCity(aS[1]);
        } else if (aS[0].equals("WorkPhone")) {
            changeWorkPhone(aS[1]);
        } else if (aS[0].equals("PrivatePhone")) {
            changePrivatePhone(aS[1]);
        } else if (aS[0].equals("Email")) {
            changeEmail(aS[1]);
        } else {
            System.out.println("Unknown target in list: "+aS[0]);
        }

    }

    private void changeSalary(String data){
        salaryTextField.setText(data);
    }
   
    private void changePerson(String data){
        nameTextField.setText(data);
    }
    private void changeAddress(String data){
        addressTextField.setText(data);
    }
    private void changePostalCode(String data){
        postalCodeTextField.setText(data);
    }
    private void changeCity(String data){
        cityTextField.setText(data);
    }
    private void changeWorkPhone(String data){
        phoneTextField.setText(data);
    }
    private void changePrivatePhone(String data){
        privatePhoneTextField.setText(data);
    }
    
    private void changeEmail(String data){
        emailTextField.setText(data);
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
