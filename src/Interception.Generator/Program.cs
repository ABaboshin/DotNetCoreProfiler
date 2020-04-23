using Interception.Common;
using Newtonsoft.Json;
using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Text;

namespace Interception.Generator
{
    class Program
    {
        static void Main(string[] args)
        {
            var interceptions = typeof(InterceptorBase)
                .Assembly
                .GetTypes()
                .SelectMany(type => type.GetRuntimeMethods())
                .Where(method => method.GetCustomAttributes<InterceptAttribute>(false).Any())
                .SelectMany(method => method.GetCustomAttributes<InterceptAttribute>(false).Select(attribute => new { method, attribute }))
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

                    return new {
                        info.attribute.CallerAssembly,
                        Target = new {
                            AssemblyName = info.attribute.TargetAssemblyName,
                            MethodName = info.attribute.TargetMethodName,
                            TypeName = info.attribute.TargetTypeName,
                            MethodParametersCount = info.attribute.TargetMethodParametersCount,
                        },
                        Interceptor = new {
                            TypeName = info.method.DeclaringType.FullName,
                            MethodName = info.method.Name,
                            AssemblyName = info.method.DeclaringType.Assembly.GetName().Name,
                            AssemblyPath = "/profiler/Interception.dll",
                            Signature = string.Join(" ", signatureBytes.Select(b => b.ToString("X2")))
                        }
                    };
                })
                .ToList();

            var initializer = typeof(InterceptorBase)
                .Assembly
                .GetTypes()
                .Where(type => type.GetCustomAttributes<InitializeAttribute>(false).Any())
                .SelectMany(type => type.GetCustomAttributes<InitializeAttribute>(false).Select(attribute => new { type, attribute }))
                .Select(info =>
                {
                    return new
                    {
                        TypeName = info.type.FullName,
                        AssemblyName = info.type.Assembly.GetName().Name,
                        AssemblyPath = "/profiler/Interception.dll",
                    };
                })
                .FirstOrDefault();

            var serializerSettings = new JsonSerializerSettings
            {
                NullValueHandling = NullValueHandling.Ignore,
                Formatting = Formatting.Indented
            };

            var json = JsonConvert.SerializeObject(new { interceptions, initializer }, serializerSettings);

            File.WriteAllText("interceptions.json", json, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
        }
    }
}
