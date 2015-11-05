/* AnaBeamDialog.java
 *
 *  Copyright (C) 2002-2007
 *  ASTRON (Netherlands Foundation for Research in Astronomy)
 *  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

package nl.astron.lofar.sas.otbcomponents;
import javax.swing.border.TitledBorder;
import nl.astron.lofar.lofarutils.LofarUtils;
import nl.astron.lofar.sas.otb.MainFrame;
import nl.astron.lofar.sas.otb.objects.AnaBeam;
import org.apache.log4j.Logger;


/**
 *
 * @created 11-02-2008, 13:35
 *
 * @author  coolen
 *
 * @version $Id$
 */
public class AnaBeamDialog extends javax.swing.JDialog {
    static Logger logger = Logger.getLogger(AnaBeamDialog.class);
    static String name = "AnaBeamDialog";
    
    /** Creates new form BeanForm
     *
     * @param   parent                  Frame where this dialog belongs
     * @param   modal                   Should the Dialog be modal or not
     * @param   selection               Vector of all analog beam params
     * @param   directionTypeChoices    String with all possible choices + default for combobox
     * @param   rankChoices             String with all possible choices + default for combobox
     * @param   edit                    indicates edit or add mode
     */
    public AnaBeamDialog(java.awt.Frame parent, boolean modal,AnaBeam anAnaBeam,boolean edit ) {

        super(parent, modal);
        initComponents();
        LofarUtils.setPopupComboChoices(inputDirectionType,anAnaBeam.getDirectionTypeChoices());
        LofarUtils.setPopupComboChoices(inputRank,anAnaBeam.getRankChoices());
        itsAnaBeam = anAnaBeam;
        editting=edit;
        initialize();
    }
    

    private void initialize() {

        inputDirectionType.setSelectedItem(itsAnaBeam.getDirectionType());
        inputAngle1.setText(itsAnaBeam.getAngle1());
        inputAngle2.setText(itsAnaBeam.getAngle2());
        coordTypeChange.setSelectedItem(itsAnaBeam.getCoordType());
        inputRank.setSelectedItem(itsAnaBeam.getRank());
    }
    
    public boolean hasChanged() {
        return isChanged;
    }
    

    public void setBorderTitle(String text) {
        TitledBorder aBorder=(TitledBorder)jPanel1.getBorder();
        aBorder.setTitle(text);
    }
    
    
    
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanel1 = new javax.swing.JPanel();
        inputDirectionType = new javax.swing.JComboBox();
        labelAngle1 = new javax.swing.JLabel();
        inputAngle1 = new javax.swing.JTextField();
        labelAngle2 = new javax.swing.JLabel();
        inputAngle2 = new javax.swing.JTextField();
        labelRank = new javax.swing.JLabel();
        cancelButton = new javax.swing.JButton();
        saveButton = new javax.swing.JButton();
        labelDirectionType = new javax.swing.JLabel();
        inputRank = new javax.swing.JComboBox();
        coordTypeChange = new javax.swing.JComboBox();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("LOFAR View TreeInfo");
        setModal(true);
        setName("loadFileDialog"); // NOI18N
        setResizable(false);
        getContentPane().setLayout(new org.netbeans.lib.awtextra.AbsoluteLayout());

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Edit/Add Analog Beam", javax.swing.border.TitledBorder.CENTER, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 1, 11))); // NOI18N

        inputDirectionType.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1" }));

        labelAngle1.setText("Angle 1:");

        labelAngle2.setText("Angle 2 :");

        labelRank.setText("Rank :");

        cancelButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_cancel.png"))); // NOI18N
        cancelButton.setText("Cancel");
        cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                cancelButtonActionPerformed(evt);
            }
        });

        saveButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_apply.png"))); // NOI18N
        saveButton.setText("Save");
        saveButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                saveButtonActionPerformed(evt);
            }
        });

        labelDirectionType.setText("directionTypes :");

        inputRank.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1" }));

        coordTypeChange.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "rad", "deg", "hmsdms", "dmsdms" }));
        coordTypeChange.setToolTipText("set to alternative coordinates");
        coordTypeChange.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                coordTypeChangeActionPerformed(evt);
            }
        });

        org.jdesktop.layout.GroupLayout jPanel1Layout = new org.jdesktop.layout.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                            .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.TRAILING, false)
                                .add(org.jdesktop.layout.GroupLayout.LEADING, labelAngle2, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .add(org.jdesktop.layout.GroupLayout.LEADING, labelAngle1, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                            .add(labelDirectionType))
                        .add(20, 20, 20)
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING, false)
                            .add(inputDirectionType, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 75, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(inputAngle2, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 185, Short.MAX_VALUE)
                            .add(inputAngle1))
                        .add(26, 26, 26)
                        .add(coordTypeChange, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .addContainerGap())
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(cancelButton)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(saveButton)
                        .addContainerGap(270, Short.MAX_VALUE))
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(labelRank, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, 93, Short.MAX_VALUE)
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(inputRank, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 75, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                        .add(266, 266, 266))))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(jPanel1Layout.createSequentialGroup()
                .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                            .add(inputDirectionType, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(labelDirectionType))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                            .add(labelAngle1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 14, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(inputAngle1, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED)
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                            .add(labelAngle2)
                            .add(inputAngle2, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE))
                        .add(31, 31, 31)
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                            .add(inputRank, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                            .add(labelRank))
                        .addPreferredGap(org.jdesktop.layout.LayoutStyle.RELATED, 39, Short.MAX_VALUE)
                        .add(jPanel1Layout.createParallelGroup(org.jdesktop.layout.GroupLayout.BASELINE)
                            .add(cancelButton)
                            .add(saveButton)))
                    .add(jPanel1Layout.createSequentialGroup()
                        .add(35, 35, 35)
                        .add(coordTypeChange, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap())
        );

        getContentPane().add(jPanel1, new org.netbeans.lib.awtextra.AbsoluteConstraints(0, 0, 460, 220));

        pack();
    }// </editor-fold>//GEN-END:initComponents
    
    private void saveButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_saveButtonActionPerformed
        //check if input is correct
        checkChanged();
        setVisible(false);
        dispose();
    }//GEN-LAST:event_saveButtonActionPerformed
    
    private void checkChanged() {
        if (!itsAnaBeam.getDirectionType().equals(inputDirectionType.getSelectedItem().toString())) {
            isChanged=true;
        }
        if (!itsAnaBeam.getAngle1().equals(inputAngle1.getText())) {
            isChanged=true;
        }
        if (!itsAnaBeam.getAngle2().equals(inputAngle2.getText())) {
            isChanged=true;
        }
        if (!itsAnaBeam.getCoordType().equals(coordTypeChange.getSelectedItem().toString())) {
            isChanged=true;
        }
        if (!itsAnaBeam.getRank().equals(inputRank.getSelectedItem().toString())) {
            isChanged=true;
        }
    }
    
    public AnaBeam getAnaBeam() {

        itsAnaBeam.setDirectionType(inputDirectionType.getSelectedItem().toString());
        itsAnaBeam.setAngle1(inputAngle1.getText());
        itsAnaBeam.setAngle2(inputAngle2.getText());
        itsAnaBeam.setCoordType(coordTypeChange.getSelectedItem().toString());
        itsAnaBeam.setRank(inputRank.getSelectedItem().toString());

        
        return itsAnaBeam;
    }
    
    private void cancelButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_cancelButtonActionPerformed
        isChanged=false;
        setVisible(false);
        dispose();
    }//GEN-LAST:event_cancelButtonActionPerformed

    private void coordTypeChangeActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_coordTypeChangeActionPerformed

        if (evt.getActionCommand().equals("comboBoxChanged")) {
            String ch = coordTypeChange.getSelectedItem().toString();
            String tmpch = ch;
            String tmpcoord = itsAnaBeam.getCoordType();
            if ((inputAngle1.getText().isEmpty() && inputAngle1.getText().isEmpty()) || ch.equals(itsAnaBeam.getCoordType())) {
                return;
            }
            try {
                if (!inputAngle1.getText().isEmpty()) {
                    switch (ch) {
                        case "dmsdms":
                            tmpch = "dms";
                            break;
                        case "hmsdms":
                            tmpch = "hms";
                            break;
                    }
                    switch (itsAnaBeam.getCoordType()) {
                        case "hmsdms":
                            tmpcoord = "hms";
                            break;
                        case "dmsdms":
                            tmpcoord = "dms";
                            break;
                    }
                    inputAngle1.setText(LofarUtils.changeCoordinate(tmpcoord, tmpch, inputAngle1.getText()));
                }
                if (!inputAngle2.getText().isEmpty()) {
                    if (ch.equals("dmsdms") || ch.equals("hmsdms")) {
                        tmpch = "dms";
                    }
                    if (itsAnaBeam.getCoordType().equals("hmsdms") || itsAnaBeam.getCoordType().equals("dmsdms")) {
                        tmpcoord = "dms";
                    }
                    inputAngle2.setText(LofarUtils.changeCoordinate(tmpcoord, tmpch, inputAngle2.getText()));
                }
                itsAnaBeam.setCoordType(ch);
            } catch (NumberFormatException ex) {
                String aS = "Error in angle input format :" + ex;
                logger.error(aS);
                LofarUtils.showErrorPanel(this, aS, new javax.swing.ImageIcon(getClass().getResource("/nl/astron/lofar/sas/otb/icons/16_warn.gif")));
            }
        }
}//GEN-LAST:event_coordTypeChangeActionPerformed
    
        
    private MainFrame itsMainFrame = null;
    private boolean   isChanged=false;
    private AnaBeam   itsAnaBeam;
    private boolean   editting          = false;
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton cancelButton;
    private javax.swing.JComboBox coordTypeChange;
    private javax.swing.JTextField inputAngle1;
    private javax.swing.JTextField inputAngle2;
    private javax.swing.JComboBox inputDirectionType;
    private javax.swing.JComboBox inputRank;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JLabel labelAngle1;
    private javax.swing.JLabel labelAngle2;
    private javax.swing.JLabel labelDirectionType;
    private javax.swing.JLabel labelRank;
    private javax.swing.JButton saveButton;
    // End of variables declaration//GEN-END:variables
    
}
