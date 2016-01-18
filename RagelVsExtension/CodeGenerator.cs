﻿using System;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.TextTemplating.VSHost;
using System.Text;
using System.Linq;
using System.Diagnostics;
using System.Threading;
using System.Reflection;
using System.IO;

namespace RagelVsExtension
{
    [Guid("3BFDBC30-23A1-461D-82E1-AD888F356625")]
    public class CodeGenerator : BaseCodeGeneratorWithSite
    {
        public override string GetDefaultExtension()
        {
            return ".cs";
        }

        private const string warning =
@"// <auto-generated>
//
//      This code was auto-generated on {0}.
//
//      DO NOT EDIT THIS FILE.
//
//      Changes to this file may cause incorrect behaviour and will be lost if
//      the code is regenerated.
//
// </auto-generated>
";
        public static string AssemblyDirectory
        {
            get
            {
                var codeBase = Assembly.GetExecutingAssembly().CodeBase;
                var uri = new UriBuilder(codeBase);
                var path = Uri.UnescapeDataString(uri.Path);
                return Path.GetDirectoryName(path);
            }
        }

        protected override byte[] GenerateCode(string inputFileName, string inputFileContent)
        {
            return Generate(inputFileName, inputFileContent, DateTime.Now);
        }

        // This method is used by tests
        public byte[] Generate(string inputFileName, string inputFileContent, DateTime dateTime)
        {
            try
            {
                var startInfo = new ProcessStartInfo
                {
                    CreateNoWindow = true,
                    Arguments = "-A -c -i " + inputFileName,
                    RedirectStandardInput = true,
                    RedirectStandardOutput = true,
                    RedirectStandardError = true,
                    StandardErrorEncoding = Encoding.UTF8,
                    StandardOutputEncoding = Encoding.UTF8,
                    UseShellExecute = false,
                    FileName = Path.Combine(AssemblyDirectory, "Ragel.exe")
                };

                var timeout = 10000;

                var hasError = false;

                var output = new StringBuilder();
                var error = new StringBuilder();

                using (var process = new Process { StartInfo = startInfo, EnableRaisingEvents = true })
                using (var outputWaitHandle = new AutoResetEvent(false))
                using (var errorWaitHandle = new AutoResetEvent(false))
                {
                    var outputDelegate = GetDataReader(output, outputWaitHandle);
                    var errorDelegate = GetDataReader(error, errorWaitHandle);
                    process.OutputDataReceived += outputDelegate;
                    process.ErrorDataReceived += errorDelegate;
                    process.Start();

                    process.BeginOutputReadLine();
                    process.BeginErrorReadLine();

                    process.StandardInput.Write(inputFileContent);
                    process.StandardInput.Flush();
                    process.StandardInput.Close();

                    if (process.WaitForExit(timeout) &&
                        outputWaitHandle.WaitOne(timeout) &&
                        errorWaitHandle.WaitOne(timeout))
                    {
                        if (process.ExitCode != 0)
                        {
                            hasError = true;
                        }
                    }
                    else
                    {
                        hasError = true;
                        error.AppendLine("Timeout expired");
                    }

                    process.OutputDataReceived -= outputDelegate;
                    process.ErrorDataReceived -= errorDelegate;
                }

                if (hasError)
                {
                    return Encoding.UTF8.GetBytes(CreateErrorDirective(error.ToString()));
                }

                var outputBytes = Encoding.UTF8.GetBytes(output.ToString());
                var warningBytes = Encoding.UTF8.GetBytes(string.Format(warning, dateTime.ToString("f")));

                var result = new byte[warningBytes.Length + outputBytes.Length];
                Array.Copy(warningBytes, result, warningBytes.Length);
                Array.Copy(outputBytes, 0, result, warningBytes.Length, outputBytes.Length);

                return result;
            }
            catch (Exception ex)
            {
                return Encoding.UTF8.GetBytes(CreateErrorDirective(ex.ToString()));
            }
        }

        private DataReceivedEventHandler GetDataReader(StringBuilder sb, AutoResetEvent handle)
        {
            return (s, e) =>
            {
                if (e.Data == null)
                {
                    handle.Set();
                }
                else
                {
                    if (sb.Length > 0)
                    {
                        sb.Append(Environment.NewLine);
                    }
                    sb.Append(e.Data);
                }
            };
        }

        private static string CreateErrorDirective(string message)
        {
            var split = message.Split(new[] { '\n' }, StringSplitOptions.RemoveEmptyEntries);
            if (split.Any())
            {
                return string.Format("#error Generation failed: {0}\n/*\n{1}\n*/", split[0], message);
            }
            return "#error Generation failed: unknown error";
        }

        private static byte[] signature = new byte[] { 0xEF, 0xBB, 0xBF };

        private static void ReplaceBOMSignatureForTheLine(byte[] output, int offset)
        {
            if (signature.Length + offset > output.Length)
            {
                return;
            }
            for (var i = 0; i < signature.Length; i++)
            {
                if (output[i + offset] != signature[i])
                {
                    return;
                }
            }

            for (var i = 0; i < signature.Length; i++)
            {
                output[i + offset] = 0x20;
            }
        }

        private static int FindNextLineStart(byte[] output, int offset)
        {
            var result = Array.IndexOf<byte>(output, 0xA, offset);
            return result + 1;
        }

        private static void ReplaceBOMSignature(byte[] output)
        {
            var index = 0;
            do
            {
                ReplaceBOMSignatureForTheLine(output, index);
                index = FindNextLineStart(output, index);
            } while (index > 0);
        }
    }
}
