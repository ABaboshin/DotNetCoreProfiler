using Interception.Extensions;
using System;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;

namespace Interception
{
    public class Loader
    {
        public Loader()
        {
            var interceptionDlls = Environment.GetEnvironmentVariable("PROFILER_INTERCEPTION_DLLS");
            Console.WriteLine($"Interception.Loader {interceptionDlls}");
            foreach (var dll in interceptionDlls.Split(new char[] { ',' }))
            {
                Console.WriteLine($"Load {dll}");
                var assembly = Assembly.LoadFrom(dll);

                var interceptors = assembly
                    .GetTypes()
                    .SelectMany(type => type.GetRuntimeMethods())
                    .SelectMany(method => method.GetCustomAttributes().Select(attribute => new { method, attribute }))
                    .Where(info => info.attribute.GetType().FullName == typeof(InterceptAttribute).FullName)
                    .Select(info =>
                    {
                        var returnType = info.method.ReturnType;
                        var parameters = info.method.GetParameters().Select(p => p.ParameterType).ToArray();
                        var signatureHelper = SignatureHelper.GetMethodSigHelper(info.method.CallingConvention, returnType);
                        signatureHelper.AddArguments(parameters, requiredCustomModifiers: null, optionalCustomModifiers: null);
                        var signatureBytes = signatureHelper.GetSignature();

                        if (info.method.IsGenericMethod)
                        {
                            byte IMAGE_CEE_CS_CALLCONV_GENERIC = 0x10;
                            var genericArguments = info.method.GetGenericArguments();

                            var newSignatureBytes = new byte[signatureBytes.Length + 1];
                            newSignatureBytes[0] = (byte)(signatureBytes[0] | IMAGE_CEE_CS_CALLCONV_GENERIC);
                            newSignatureBytes[1] = (byte)genericArguments.Length;
                            Array.Copy(signatureBytes, 1, newSignatureBytes, 2, signatureBytes.Length - 1);

                            signatureBytes = newSignatureBytes;
                        }

                        int size = Marshal.SizeOf(signatureBytes[0]) * signatureBytes.Length;
                        IntPtr ptr = Marshal.AllocHGlobal(size);
                        Marshal.Copy(signatureBytes, 0, ptr, signatureBytes.Length);

                        return new ImportInterception
                        {
                            CallerAssembly = info.attribute.GetPropertyValue<string>("CallerAssembly"),
                            TargetAssemblyName = info.attribute.GetPropertyValue<string>("TargetAssemblyName"),
                            TargetMethodName = info.attribute.GetPropertyValue<string>("TargetMethodName"),
                            TargetTypeName = info.attribute.GetPropertyValue<string>("TargetTypeName"),
                            TargetMethodParametersCount = info.attribute.GetPropertyValue<int>("TargetMethodParametersCount"),
                            InterceptorTypeName = info.method.DeclaringType.FullName,
                            InterceptorMethodName = info.method.Name,
                            InterceptorAssemblyName = info.method.DeclaringType.Assembly.GetName().Name,
                            Signature = ptr,
                            SignatureLength = signatureBytes.Length
                        };
                    })
                    .ToList();

                foreach (var interceptor in interceptors)
                {
                    Console.WriteLine($"{interceptor.CallerAssembly} {interceptor.TargetAssemblyName} {interceptor.TargetMethodName} {interceptor.TargetTypeName} {interceptor.TargetMethodParametersCount} ");
                    NativeMethods.AddInterceptor(interceptor);
                }

                var initializers = assembly
                    .GetTypes()
                    .Where(type => type.GetCustomAttributes().Where(a => a.GetType().FullName == typeof(InterceptAttribute).FullName).Any())
                    .ToList();

                foreach (var initializer in initializers)
                {
                    Activator.CreateInstance(initializer);
                }
            }
        }
    }
}
