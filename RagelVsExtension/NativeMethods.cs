using System;
using System.Runtime.InteropServices;
using System.Text;

namespace RagelVsExtension
{
    public static class NativeMethods
    {
        [DllImport("Ragel.dll", 
            CharSet = CharSet.Unicode,
            CallingConvention = CallingConvention.StdCall, 
            EntryPoint = "process")]
        [return: MarshalAs(UnmanagedType.LPWStr)]
        public static extern string Process(string inputFileName, string inputContent, 
            [MarshalAs(UnmanagedType.Bool)]out bool hasErrors);
    }
}
