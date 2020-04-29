using MassTransit;
using Microsoft.AspNetCore;
using Microsoft.AspNetCore.Hosting;
using SDILReader;
using System;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.CompilerServices;
using System.Threading.Tasks;

namespace SampleApp
{
    public class Program
    {
        public static Task Test<T>(IConsumer<T> consumer, ConsumeContext<T> context) where T : class
        {
            return Task.CompletedTask;
        }

        public static void Main(string[] args)
        {
            //Test();
            //var methods = typeof(Program).GetMethods();
            //var method = methods.Where(m => m.Name == "Test").First();

            //var returnType = method.ReturnType;
            //var parameters = method.GetParameters().Select(p => p.ParameterType).ToArray();
            //var signatureHelper = SignatureHelper.GetMethodSigHelper(method.CallingConvention, returnType);
            //signatureHelper.AddArguments(parameters, requiredCustomModifiers: null, optionalCustomModifiers: null);
            //var signatureBytes = signatureHelper.GetSignature();

            //if (method.IsGenericMethod)
            //{
            //    byte IMAGE_CEE_CS_CALLCONV_GENERIC = 0x10;
            //    var genericArguments = method.GetGenericArguments();

            //    var newSignatureBytes = new byte[signatureBytes.Length + 1];
            //    newSignatureBytes[0] = (byte)(signatureBytes[0] | IMAGE_CEE_CS_CALLCONV_GENERIC);
            //    newSignatureBytes[1] = (byte)genericArguments.Length;
            //    Array.Copy(signatureBytes, 1, newSignatureBytes, 2, signatureBytes.Length - 1);

            //    signatureBytes = newSignatureBytes;
            //}

            //foreach (var b in signatureBytes)
            //{
            //    Console.WriteLine(b.ToString("X2"));
            //}


            CreateWebHostBuilder(args).Build().Run();
        }

        public Program(string str)
        {

        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]

        public static void Test()
        {
            var test = (Program)Activator.CreateInstance(typeof(Program), "xxx");
            var data = new object[] { "test" };
            var assembiles = AppDomain.CurrentDomain.GetAssemblies();
            var types = typeof(Program).Assembly.GetTypes();
            var type = types.Where(t => t.Name == "__InterceptionDllLoaderClass__").FirstOrDefault();
            var method = type.GetMethod("__InterceptionDllLoaderMethod__", BindingFlags.Public | BindingFlags.Static);

            Globals.LoadOpCodes();
            var mi = typeof(Program).GetMethod("Main");
            Dump(typeof(Program).GetMethod("Main"));
            Console.WriteLine();
            Dump(method);
        }

        private static void Dump(MethodInfo mi)
        {
            var mr = new MethodBodyReader(mi);
            string msil = mr.GetBodyCode();
            Console.WriteLine(msil);
        }

        public static IWebHostBuilder CreateWebHostBuilder(string[] args) { 
            return WebHost.CreateDefaultBuilder(args)
                .UseStartup<Startup>();
        }
    }
}
