using Newtonsoft.Json;
using System;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Text;
using Wrapper;
using Wrapper.Common;

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
                .Select(method =>
                {
                    var attribute = method.GetCustomAttribute<InterceptAttribute>(false);
                    var returnType = method.ReturnType;
                    var parameters = method.GetParameters().Select(p => p.ParameterType).ToArray();
                    var signatureHelper = SignatureHelper.GetMethodSigHelper(method.CallingConvention, returnType);
                    signatureHelper.AddArguments(parameters, requiredCustomModifiers: null, optionalCustomModifiers: null);
                    var signatureBytes = signatureHelper.GetSignature();

                    if (method.IsGenericMethod)
                    {
                        byte IMAGE_CEE_CS_CALLCONV_GENERIC = 0x10;
                        var genericArguments = method.GetGenericArguments();

                        var newSignatureBytes = new byte[signatureBytes.Length + 1];
                        newSignatureBytes[0] = (byte)(signatureBytes[0] | IMAGE_CEE_CS_CALLCONV_GENERIC);
                        newSignatureBytes[1] = (byte)genericArguments.Length;
                        Array.Copy(signatureBytes, 1, newSignatureBytes, 2, signatureBytes.Length - 1);

                        signatureBytes = newSignatureBytes;
                    }

                    return new {
                        attribute.CallerAssembly,
                        attribute.TargetAssemblyName,
                        attribute.TargetMethodName,
                        attribute.TargetTypeName,
                        WrapperTypeName = method.DeclaringType.FullName,
                        WrapperMethodName = method.Name,
                        WrapperAssemblyName = method.DeclaringType.Assembly.GetName().Name,
                        WrapperAssemblyPath = "/profiler/Wrapper.dll",
                        WrapperSignature = string.Join(" ", signatureBytes.Select(b => b.ToString("X2")))
                    };
                })
                .ToList();


            var serializerSettings = new JsonSerializerSettings
            {
                NullValueHandling = NullValueHandling.Ignore,
                Formatting = Formatting.Indented
            };

            var json = JsonConvert.SerializeObject(interceptions, serializerSettings);
            Console.WriteLine(json);

            File.WriteAllText("interceptions.json", json, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
        }
    }
}
