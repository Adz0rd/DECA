namespace DECA
{
    partial class Main
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.analysisProgressBar = new System.Windows.Forms.ProgressBar();
            this.label1 = new System.Windows.Forms.Label();
            this.driveSelectionBox = new System.Windows.Forms.ComboBox();
            this.analysisBeginButton = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.statusLabel = new System.Windows.Forms.Label();
            this.scanSectorBackgroundWorker = new System.ComponentModel.BackgroundWorker();
            this.label5 = new System.Windows.Forms.Label();
            this.quickSearchCheckBox = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // analysisProgressBar
            // 
            this.analysisProgressBar.Location = new System.Drawing.Point(12, 182);
            this.analysisProgressBar.Maximum = 10000;
            this.analysisProgressBar.Name = "analysisProgressBar";
            this.analysisProgressBar.Size = new System.Drawing.Size(372, 23);
            this.analysisProgressBar.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(160, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Step 1. Select a drive to analyse";
            // 
            // driveSelectionBox
            // 
            this.driveSelectionBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.driveSelectionBox.FormattingEnabled = true;
            this.driveSelectionBox.Location = new System.Drawing.Point(265, 6);
            this.driveSelectionBox.Name = "driveSelectionBox";
            this.driveSelectionBox.Size = new System.Drawing.Size(119, 21);
            this.driveSelectionBox.TabIndex = 1;
            // 
            // analysisBeginButton
            // 
            this.analysisBeginButton.Location = new System.Drawing.Point(265, 41);
            this.analysisBeginButton.Name = "analysisBeginButton";
            this.analysisBeginButton.Size = new System.Drawing.Size(119, 23);
            this.analysisBeginButton.TabIndex = 2;
            this.analysisBeginButton.Text = "Begin analysis";
            this.analysisBeginButton.UseVisualStyleBackColor = true;
            this.analysisBeginButton.Click += new System.EventHandler(this.analysisBeginButton_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(12, 46);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(201, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "Step 2. Press the \"Begin analysis\" button";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 78);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(210, 26);
            this.label3.TabIndex = 5;
            this.label3.Text = "Step 3. Choose if you would like to make it \r\na \"Quick Search\"";
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(12, 166);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(40, 13);
            this.label4.TabIndex = 6;
            this.label4.Text = "Status:";
            // 
            // statusLabel
            // 
            this.statusLabel.AutoSize = true;
            this.statusLabel.Location = new System.Drawing.Point(58, 166);
            this.statusLabel.Name = "statusLabel";
            this.statusLabel.Size = new System.Drawing.Size(49, 13);
            this.statusLabel.TabIndex = 7;
            this.statusLabel.Text = "waiting...";
            // 
            // scanSectorBackgroundWorker
            // 
            this.scanSectorBackgroundWorker.WorkerReportsProgress = true;
            this.scanSectorBackgroundWorker.DoWork += new System.ComponentModel.DoWorkEventHandler(this.scanSectorBackgroundWorker_DoWork);
            this.scanSectorBackgroundWorker.ProgressChanged += new System.ComponentModel.ProgressChangedEventHandler(this.scanSectorBackgroundWorker_ProgressChanged);
            this.scanSectorBackgroundWorker.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.scanSectorBackgroundWorker_RunWorkerCompleted);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(12, 116);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(369, 26);
            this.label5.TabIndex = 8;
            this.label5.Text = "Step 4. Monitor the progress bar below and wait for the analysis to complete. \r\nO" +
    "nce the analysis is complete, a simple results window will be displayed. ";
            // 
            // quickSearchCheckBox
            // 
            this.quickSearchCheckBox.AutoSize = true;
            this.quickSearchCheckBox.Location = new System.Drawing.Point(265, 77);
            this.quickSearchCheckBox.Name = "quickSearchCheckBox";
            this.quickSearchCheckBox.Size = new System.Drawing.Size(91, 17);
            this.quickSearchCheckBox.TabIndex = 9;
            this.quickSearchCheckBox.Text = "Quick Search";
            this.quickSearchCheckBox.UseVisualStyleBackColor = true;
            // 
            // Main
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(396, 217);
            this.Controls.Add(this.quickSearchCheckBox);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.statusLabel);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.analysisBeginButton);
            this.Controls.Add(this.driveSelectionBox);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.analysisProgressBar);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "Main";
            this.ShowIcon = false;
            this.Text = "DECA - Drive erasure confidence application";
            this.Load += new System.EventHandler(this.Main_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ProgressBar analysisProgressBar;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ComboBox driveSelectionBox;
        private System.Windows.Forms.Button analysisBeginButton;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label statusLabel;
        private System.ComponentModel.BackgroundWorker scanSectorBackgroundWorker;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.CheckBox quickSearchCheckBox;
    }
}

