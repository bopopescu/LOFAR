/*
 * PlotTestFrameStandAlone.java
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
 *
 */

package nl.astron.lofar.java.gui.plotter.test;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Toolkit;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import nl.astron.lofar.java.gui.plotter.PlotConstants;
import nl.astron.lofar.java.gui.plotter.PlotPanel;
import nl.astron.lofar.java.gui.plotter.exceptions.PlotterException;

/**
 * Test class for TestData-SGT combination of plotting. 
 * It can be used with no external dependencies.<br><br>
 * Please check that, in the plotter_config.properties file, the variable DATA_ACCESS_CLASS
 * is set to nl.astron.lofar.java.gui.plotter.test.PlotDataAccessTestImpl
 * 
 * @created 05-05-2006, 13:00
 * @author pompert
 * @version $Id$
 * @see PlotDataAccessTestImpl
 * @see plotter_config.properties
 */
public class PlotTestFrameStandAlone extends javax.swing.JFrame {
    
    private PlotPanel testPanel;
    JScrollPane legendPane;
    private JLabel exceptionLabel;
    private boolean plotPresent;
    
    /** Creates new form PlotTestFrameStandAlone */
    public PlotTestFrameStandAlone() {
        
        plotPresent = false;
        //For use with PlotDataAccessTestImpl
        //testPanel.createPlot(PlotConstants.PLOT_XYLINE,"line");
        //testPanel2.createPlot(PlotConstants.PLOT_GRID,"grid");
        
        //For use with PlotDataAccessParmDBImpl class
        //testPanel.createPlot(PlotConstants.PLOT_XYLINE,"parm*");
        //testPanel2.createPlot(PlotConstants.PLOT_GRID,"parm2");
        
         
         //this.getContentPane().add(testPanel2.getPlot(),new GridBagConstraints(1,0,1,1,1,1,GridBagConstraints.CENTER,GridBagConstraints.BOTH,new Insets(0,0,0,0),1,1));
        //this.getContentPane().add(testPanel2.getLegendForPlot(),new GridBagConstraints(1,1,1,1,1,1,GridBagConstraints.CENTER,GridBagConstraints.BOTH,new Insets(0,0,0,0),1,1));
        
        
        this.setSize(new Dimension(640,480));
        initComponents();
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        plotModPanel = new javax.swing.JPanel();
        jSeparator1 = new javax.swing.JSeparator();
        cParamConstraint = new javax.swing.JComboBox();
        cLegend = new javax.swing.JCheckBox();
        bplotButton = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setTitle("LOFAR | JAVA Plotter Test Application");
        setIconImage(getIconImage());
        plotModPanel.setLayout(new java.awt.GridBagLayout());

        jSeparator1.setOrientation(javax.swing.SwingConstants.VERTICAL);
        jSeparator1.setMinimumSize(new java.awt.Dimension(50, 10));
        plotModPanel.add(jSeparator1, new java.awt.GridBagConstraints());

        cParamConstraint.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "line", "grid" }));
        plotModPanel.add(cParamConstraint, new java.awt.GridBagConstraints());

        cLegend.setText("Legend");
        cLegend.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        cLegend.setHorizontalTextPosition(javax.swing.SwingConstants.LEADING);
        cLegend.setMargin(new java.awt.Insets(0, 0, 0, 0));
        plotModPanel.add(cLegend, new java.awt.GridBagConstraints());

        bplotButton.setText("Plot");
        bplotButton.setHorizontalAlignment(javax.swing.SwingConstants.RIGHT);
        bplotButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                handlePlotButton(evt);
            }
        });

        plotModPanel.add(bplotButton, new java.awt.GridBagConstraints());

        getContentPane().add(plotModPanel, java.awt.BorderLayout.NORTH);

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void handlePlotButton(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_handlePlotButton
        JButton source = (JButton)evt.getSource();
        if(source == bplotButton){
                String[] argsForController = new String[1];
                argsForController[0] = cParamConstraint.getSelectedItem().toString();
                               
                //plotPane.add(testPanel.getPlot(),new GridBagConstraints(0,0,1,1,1,1,GridBagConstraints.CENTER,GridBagConstraints.BOTH,new Insets(0,0,0,0),1,1));
                //plotPane.add(testPanel.getLegendForPlot(),new GridBagConstraints(0,1,1,1,1,1,GridBagConstraints.CENTER,GridBagConstraints.BOTH,new Insets(0,0,0,0),1,1));
                if(plotPresent){
                    try {
                        this.getContentPane().remove(testPanel);
                        this.getContentPane().remove(legendPane);
                    }catch(Exception e){
                        
                    }
                }
                try {
                    if(exceptionLabel !=null){
                        //this.getContentPane().remove(exceptionLabel);
                        exceptionLabel = null;
                    }
                    testPanel = new PlotPanel();
                    if(argsForController[0].equalsIgnoreCase("line")){
                        testPanel.createPlot(PlotConstants.PLOT_XYLINE,true,argsForController);
                    }else if(argsForController[0].equalsIgnoreCase("grid")){
                        testPanel.createPlot(PlotConstants.PLOT_GRID,true,argsForController);
                    }
                    this.add(testPanel,BorderLayout.CENTER);
                    
                    if(cLegend.isSelected()){
                        int legendWidth = (int)testPanel.getPreferredSize().getWidth()+10;
                        int legendHeight =  100;
                        
                        legendPane = new JScrollPane(testPanel.getLegendForPlot());
                        legendPane.setPreferredSize(new Dimension(legendWidth,legendHeight));
                        legendPane.setBackground(Color.WHITE);
                        legendPane.getViewport().setBackground(Color.WHITE);
                        this.add(legendPane,BorderLayout.SOUTH);
                    }
                    
                    plotPresent = true;
                    
                } catch (PlotterException ex) {
                    JOptionPane.showMessageDialog(this, ex.getMessage(),
                            "Error detected",
                            JOptionPane.ERROR_MESSAGE);
                    exceptionLabel = new JLabel(ex.getMessage());
                    exceptionLabel.setForeground(Color.RED);
                    //this.add(exceptionLabel,BorderLayout.CENTER);
                    plotPresent = false;
                    ex.printStackTrace();
                }
                
                this.pack();
                Dimension d = Toolkit.getDefaultToolkit().getScreenSize();
                this.setLocation(d.width/2 - this.getWidth()/2, d.height /2 - this.getHeight() /2);
            }
    }//GEN-LAST:event_handlePlotButton
    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
       PlotTestFrameStandAlone af = new PlotTestFrameStandAlone();
       Dimension d = Toolkit.getDefaultToolkit().getScreenSize();
       af.setLocation(d.width/2 - af.getWidth()/2, d.height /2 - af.getHeight() /2);
       af.setVisible(true);
    }
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton bplotButton;
    private javax.swing.JCheckBox cLegend;
    private javax.swing.JComboBox cParamConstraint;
    private javax.swing.JSeparator jSeparator1;
    private javax.swing.JPanel plotModPanel;
    // End of variables declaration//GEN-END:variables
    
}
