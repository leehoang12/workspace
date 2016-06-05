using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using putYourNamespaceNameHere;
using Excel = Microsoft.Office.Interop.Excel;
using System.IO;

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
        const string FILE_NAME = @"\Report.xls";
        const int VREF = 10;                    // Vreff of ADC
        const int MAX_RANGE = 1023;
        const int VOLT_MAX = 1120;

        string[] Title = new string[] { "Start time", "Stop time", "VDC", "TPA3", "TPB3", "TPC3", "TPD3", "TPE3", 
                                        "Tiêu chuẩn", "OK", " ", "TPA4", "TPB4", "TPC4", "TPD4", "TPE4", "Tiêu chuẩn", "OK" };

        string StringStartTime;
        string StringStopTime;
        string StringVDC;

        Excel.Application xlApp = null;
        string pathFile = Environment.GetFolderPath(Environment.SpecialFolder.Desktop) + FILE_NAME;
        int count_TP3 = 0;
        int count_TP4 = 0;

        string[] StringTP3 = new String[1000];
        string StringTP3_TieuChuan;
        string StringTP3_OK;

        string[] StringTP4 = new String[1000];
        string StringTP4_TieuChuan;
        string StringTP4_OK;

        int VDC_rawData;
        int Cap1_value_rawData;
        int Cap2_value_rawData;

        int VDC;
        int Cap1_value;
        int Cap2_value;

        int doChinhXac;
        int dienAp_TieuChuan_TP3;       // 1/3 Vdc
        int dienAp_TieuChuan_TP4;       // 2/3 Vdc

        internal const uint WM_DEVICECHANGE = 0x0219;
		// member variables ///////////////////////////////////////////////////////////////////////////
		public USBClass USBObject;

		// constructor ////////////////////////////////////////////////////////////////////////////////
		public Form1()
		{
			InitializeComponent();
			USBObject = new USBClass();
            create_file_excel();
            count_TP3 = 0;
            count_TP4 = 0;
            doChinhXac = 10;        // do chinh xac = 10%
            for (int i = 0; i < 1000; i++)
            {
                StringTP4[i] = "x";
                StringTP3[i] = "x";
            }
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
            if (USBObject.fromDeviceToHostBuffer[1] == START_TIME)          // start time
            {
                DateTime dt = DateTime.Now;
                labelStartTime.Text = dt.ToString();
                labelStopTime.Text = TIME_DEFAULT_STRING;
                StringStartTime = labelStartTime.Text;
                count_TP3 = 0;
                count_TP4 = 0;
                for (int i = 0; i < 1000; i++){
                    StringTP4[i] = "x";
                    StringTP3[i] = "x";
                }
            }
            else {
                if (USBObject.fromDeviceToHostBuffer[1] == STOP_TIME) {    // stop time
                    DateTime dt = DateTime.Now;
                    labelStopTime.Text = dt.ToString();
                    StringStopTime = labelStopTime.Text;
                    writeDataToExcelFile();
                    for (int i = 0; i < 1000; i++){
                        StringTP4[i] = "x";
                        StringTP3[i] = "x";
                    }
                    count_TP3 = 0;
                    count_TP4 = 0;
                }
            }

            VDC_rawData = USBObject.fromDeviceToHostBuffer[2] << 8 | (USBObject.fromDeviceToHostBuffer[3] & 0xFF);
            Cap1_value_rawData = USBObject.fromDeviceToHostBuffer[4] << 8 | (USBObject.fromDeviceToHostBuffer[5] & 0xFF);
            Cap2_value_rawData = USBObject.fromDeviceToHostBuffer[6] << 8 | (USBObject.fromDeviceToHostBuffer[7] & 0xFF);

            VDC = VDC_rawData;
            Cap1_value = handle_data_adc(Cap1_value_rawData);
            Cap2_value = handle_data_adc(Cap2_value_rawData);

            dienAp_TieuChuan_TP3 = VDC / 3;
            dienAp_TieuChuan_TP4 = 2*VDC / 3;

            StringVDC = Convert.ToString(VDC);
            textboxVDC.Text = StringVDC;
            StringTP3_TieuChuan = (dienAp_TieuChuan_TP3 - doChinhXac * dienAp_TieuChuan_TP3 / 100).ToString()
                                        + " - " + (dienAp_TieuChuan_TP3 + doChinhXac * dienAp_TieuChuan_TP3 / 100).ToString();
            StringTP4_TieuChuan = (dienAp_TieuChuan_TP4 - doChinhXac * dienAp_TieuChuan_TP4 / 100).ToString()
                                        + " - " + (dienAp_TieuChuan_TP4 + doChinhXac * dienAp_TieuChuan_TP4 / 100).ToString();

            StringTP3[count_TP3] = Convert.ToString(Cap1_value_rawData);
            textBox_Tu1.Text = StringTP3[count_TP3];
            count_TP3++;
            if (count_TP3 > 4)
                count_TP3 = 0;

            StringTP4[count_TP4] = Convert.ToString(Cap2_value_rawData);
            textBox_Tu2.Text = StringTP4[count_TP4];
            count_TP4++;
            if (count_TP3 > 4)
                count_TP3 = 0;

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


        private void writeDataToExcelFile()
        {


            StringTP3_OK = checkData(StringTP3, dienAp_TieuChuan_TP3 - doChinhXac * dienAp_TieuChuan_TP3 / 100, dienAp_TieuChuan_TP3 + doChinhXac * dienAp_TieuChuan_TP3 / 100, count_TP3);
            StringTP4_OK = checkData(StringTP4, dienAp_TieuChuan_TP4 - doChinhXac * dienAp_TieuChuan_TP4 / 100, dienAp_TieuChuan_TP4 + doChinhXac * dienAp_TieuChuan_TP4 / 100, count_TP4);

            String[] data = new string[] { StringStartTime, StringStopTime, StringVDC, StringTP3[0], StringTP3[1], StringTP3[2], StringTP3[3], StringTP3[4], StringTP3_TieuChuan, StringTP3_OK, "--",
                                                                                       StringTP4[0], StringTP4[1], StringTP4[2], StringTP4[3], StringTP4[4], StringTP4_TieuChuan, StringTP4_OK};

            xlApp = new Microsoft.Office.Interop.Excel.Application();       // create file excel
            if (xlApp == null)
            {
                MessageBox.Show("Excel is not properly installed!!");
                return;
            }

            Excel.Workbook xlWorkBook;
            Excel.Worksheet xlWorkSheet;
            object misValue = System.Reflection.Missing.Value;

            xlWorkBook = xlApp.Workbooks.Open(pathFile, 0, false, 5, "", "", false, Excel.XlPlatform.xlWindows, "", true, false, 0, true, false, false);
            xlWorkSheet = (Excel.Worksheet)xlWorkBook.Worksheets.get_Item(1);

            Excel.Range xlRange = (Excel.Range)xlWorkSheet.Cells[xlWorkSheet.Rows.Count, 1];
            long lastRow = (long)xlRange.get_End(Excel.XlDirection.xlUp).Row;
            long newRow = lastRow + 1;

            for (int i = 0; i < data.Length; i++)
            {
                xlWorkSheet.Cells[newRow, i + 1] = data[i];
            }
            xlWorkBook.Save();
            xlWorkBook.Close(true, pathFile);
            xlApp.Quit();

            releaseObject(xlWorkSheet);
            releaseObject(xlWorkBook);
            releaseObject(xlApp);
        }

        private string checkData(string[] TP, int min, int max, int length)
        {
            int i;

            for (i = 0; i < length; i++)
            {
                if (TP[i] != "x")
                {
                    if (Int32.Parse(TP[i]) > max || Int32.Parse(TP[i]) < min)
                    {
                        if (i > 5)
                            TP[4] = TP[i];
                        return "FAIL";
                    }
                }
            }
            return "OK";
        }

        private void create_file_excel()
        {
            xlApp = new Microsoft.Office.Interop.Excel.Application();       // create file excel
            if (xlApp == null)
            {
                MessageBox.Show("Excel is not properly installed!!");
                return;
            }

            Excel.Workbook xlWorkBook;
            Excel.Worksheet xlWorkSheet;
            object misValue = System.Reflection.Missing.Value;

            if (!File.Exists(pathFile))         // neu file chua ton tai
            {
                xlWorkBook = xlApp.Workbooks.Add(misValue);
                xlWorkSheet = (Excel.Worksheet)xlWorkBook.Worksheets.get_Item(1);
                for (int i = 0; i < Title.Length; i++)
                {
                    xlWorkSheet.Cells[1, i + 1] = Title[i];
                }
                xlWorkBook.SaveAs(pathFile, Excel.XlFileFormat.xlWorkbookNormal, misValue, misValue, misValue, misValue, Excel.XlSaveAsAccessMode.xlExclusive, misValue, misValue, misValue, misValue, misValue);
                xlWorkBook.Close(true, misValue, misValue);
            }
            else
            {
                xlWorkBook = xlApp.Workbooks.Open(pathFile, 0, false, 5, "", "", false, Excel.XlPlatform.xlWindows, "", true, false, 0, true, false, false);
                xlWorkSheet = (Excel.Worksheet)xlWorkBook.Worksheets.get_Item(1);
                for (int i = 0; i < Title.Length; i++)
                {
                    xlWorkSheet.Cells[1, i + 1] = Title[i];
                }
                xlWorkBook.Save();
                xlWorkBook.Close(true, pathFile);
            }
            //xlWorkBook.Close(true, misValue, misValue);
            xlApp.Quit();

            releaseObject(xlWorkSheet);
            releaseObject(xlWorkBook);
            releaseObject(xlApp);
            LabelSaveDir.Text = pathFile;
        }

        private void releaseObject(object obj)
        {
            try
            {
                System.Runtime.InteropServices.Marshal.ReleaseComObject(obj);
                obj = null;
            }
            catch (Exception ex)
            {
                obj = null;
                MessageBox.Show("Exception Occured while releasing object " + ex.ToString());
            }
            finally
            {
                GC.Collect();
            }
        }

        private int handle_data_adc(int raw_data) {
            double V_Current;
            V_Current = VREF * raw_data / 1023.0;
            return (int)(V_Current * VOLT_MAX / VREF); 
        }
	}		// end class
}		// end namespace

