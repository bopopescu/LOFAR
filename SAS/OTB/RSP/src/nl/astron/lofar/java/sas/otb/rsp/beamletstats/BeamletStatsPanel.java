/*
 * BeamletStatsPanel.java
 *
 * Copyright (C) 2006
 * ASTRON (Netherlands Foundation for Research in Astronomy)
 * P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 */

package nl.astron.lofar.java.sas.otb.rsp.beamletstats;

import java.util.HashMap;

import javax.swing.JPanel;

import nl.astron.lofar.java.sas.otb.rsp.ITabPanel;
import nl.astron.lofar.java.sas.otb.rsp.MainPanel;
import nl.astron.lofar.java.sas.otb.rsp.PlotDataModel;
import nl.astron.lofar.sas.otb.jrsp.RSPMask;

/**
 *
 * @author  balken
 */
public class BeamletStatsPanel extends JPanel implements ITabPanel {
    /** Reference to the MainPanel. */
    private MainPanel itsMainPanel;
    
    /** Creates new form BeamletStatsPanel */
    public BeamletStatsPanel() {
        initComponents();
    }
    
    /**
     * Used to initialize the ITabPanel and give it a refrence to the main panel. 
     * @param   mainPanel   The MainPanel.
     */
    public void init(MainPanel mainPanel) {
        itsMainPanel = mainPanel;
    }
    
    /**
     * Method that can be called by the main panel to update this panel.
     * @param   updateType  The type of update.
     */
    public void update(int updateType) {
        /**
         * If the board is connected update the plot.
         * Else disable panel with enablePanel().
         */ 
        if (itsMainPanel.getBoard().isConnected()) {
            /*
             * Construct a mask based on the currently selected board.
             */
            RSPMask mask = new RSPMask();
            mask.setBit( itsMainPanel.getSelectedBoardIndex() );
            
            
            // make hashmap to send type and data to the plotContainer
            HashMap<String,Object> hm = new HashMap<String,Object>();
            hm.put("type", PlotDataModel.BEAMLET_STATS);
            hm.put("data", itsMainPanel.getBoard().getBeamletStats(mask));
            
            plotContainer.updatePlot( hm );
        } else {
            enablePanel(false);
        }
            
    }
    
    /**
     * Method that can be called to disable or enable the board.
     * @param   b       Boolean value used to determine to enable (true) or
     *                  disable (false).
     */
    public void enablePanel(boolean b) {
        // on disable
        if (!b) {
            // create hashmap to send with updateplot
            HashMap<String,Object> hm = new HashMap<String,Object>();
            hm.put("type", PlotDataModel.BEAMLET_STATS);
            hm.put("data", new double[0]);
            plotContainer.updatePlot(hm);
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        plotContainer = new nl.astron.lofar.java.sas.otb.rsp.PlotContainer();

        org.jdesktop.layout.GroupLayout layout = new org.jdesktop.layout.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(plotContainer, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 426, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(org.jdesktop.layout.GroupLayout.LEADING)
            .add(layout.createSequentialGroup()
                .addContainerGap()
                .add(plotContainer, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE, 311, org.jdesktop.layout.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(org.jdesktop.layout.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
    }// </editor-fold>//GEN-END:initComponents
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private nl.astron.lofar.java.sas.otb.rsp.PlotContainer plotContainer;
    // End of variables declaration//GEN-END:variables
    
}
