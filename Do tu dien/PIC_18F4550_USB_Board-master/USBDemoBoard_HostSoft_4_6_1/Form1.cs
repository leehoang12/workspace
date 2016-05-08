using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using putYourNamespaceNameHere;
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace USBDemoBoard
{

	/////////////////////////////////////////////////////////////////////////////////////////////////
	public partial class Form1 : Form
	{

		// constants ////////////////////////////////////////////////////////////////////////////////

        const byte START_TIME = 0x04;
        const byte STOP_TIME = 0x02;
        const string TIME_DEFAULT_STRING = "MM/DD/YYYY H:M:S";

        internal const uint WM_DEVICECHANGE = 0x0219;
		// member variables ///////////////////////////////////////////////////////////////////////////
		public USBClass USBObject;

		// constructor ////////////////////////////////////////////////////////////////////////////////
		public Form1()
		{
			InitializeComponent();
			USBObject = new USBClass();
		}

		///////////////////////////////////////////////////////////////////////////////////////////////
		private void Form1_Load(object sender, EventArgs e)
		{
			attemptUSBConnectionFrontEnd();						// attempt USB connection
		}

		///////////////////////////////////////////////////////////////////////////////////////////////
		private void lblInfo_Click(object sender, EventArgs e)
		{
			if (USBObject.connectionState == USBClass.CONNECTION_NOT_SUCCESSFUL)
			{				// verify not already connected . . .
				attemptUSBConnectionFrontEnd();														// then attempt to connect again
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////////
		void attemptUSBConnectionFrontEnd()
		{
			lblInfo.Text = "connecting . . . ";

			USBObject.connectionState = USBObject.attemptUSBConnection();												// attempt to connect to USB board

			if (USBObject.connectionState == USBClass.CONNECTION_SUCCESSFUL)
			{								// if connection was successful
				lblInfo.BackColor = System.Drawing.Color.LimeGreen;
				lblInfo.Text = "connection successful";
				tmrUSB.Enabled = true;
			}
			else if (USBObject.connectionState == USBClass.CONNECTION_NOT_SUCCESSFUL)
			{		// else if connection was not successful
				lblInfo.BackColor = System.Drawing.Color.Red;
				lblInfo.Text = "connection not successful, click here to retry";
			}
		}

		///////////////////////////////////////////////////////////////////////////////////////////////
		private void tmrUSB_Tick(object sender, EventArgs e)
		{
			USBObject.receiveViaUSB();
            if (USBObject.fromDeviceToHostBuffer[1] == START_TIME)
            {
                DateTime dt = DateTime.Now;
                labelStartTime.Text = dt.ToString();
                labelStopTime.Text = TIME_DEFAULT_STRING;
            }
            else {
                if (USBObject.fromDeviceToHostBuffer[1] == STOP_TIME) {
                    DateTime dt = DateTime.Now;
                    labelStopTime.Text = dt.ToString();
                }
            }

            textboxVDC.Text = Convert.ToString(USBObject.fromDeviceToHostBuffer[1]);
            textBox_Tu1.Text = Convert.ToString(USBObject.fromDeviceToHostBuffer[3]);
            textBox_Tu2.Text = Convert.ToString(USBObject.fromDeviceToHostBuffer[4]);

			USBObject.sendViaUSB();
		}

        private void timer1_Tick(object sender, EventArgs e)
        {
            DateTime dt = DateTime.Now;
            labelTime.Text = dt.ToString();
        }


        protected override void WndProc(ref Message m)
        {
            if (m.Msg == WM_DEVICECHANGE) {
                attemptUSBConnectionFrontEnd();
            }
            base.WndProc(ref m);
        }
	}		// end class
}		// end namespace

