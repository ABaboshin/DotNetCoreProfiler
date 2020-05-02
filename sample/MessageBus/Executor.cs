using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Threading.Tasks;

namespace SampleApp.MessageBus
{
    public class Executor
    {
        internal static readonly ModuleBuilder Module;

        static Executor()
        {
            var asm = AssemblyBuilder.DefineDynamicAssembly(new AssemblyName("Interception"), AssemblyBuilderAccess.Run);
            Module = asm.DefineDynamicModule("DynamicModule");
        }

        public static object Execute(object _this, List<object> _parameters, MethodInfo method)
        {
            //var method = FindMethod();

            // arg count: parameters count + 1 if is instance method
            var effectiveTypes = new List<Type>();
            var parameterTypes = new List<Type>();
            var parameters = new List<object>();
            if (_this != null)
            {
                parameters.Add(_this);
                //parameterTypes.Add(method.DeclaringType);
                parameterTypes.Add(typeof(object));
                effectiveTypes.Add(method.DeclaringType);
            }

            parameters.AddRange(_parameters);
            //parameterTypes.AddRange(method.GetParameters().Select(p => p.ParameterType));
            parameterTypes.AddRange(method.GetParameters().Select(p => typeof(object)));
            effectiveTypes.AddRange(method.GetParameters().Select(p => p.ParameterType));

            Console.WriteLine($"parameters {parameters.Count()}");
            // resolve Func<T1, ... Task<TResult>>
            var genericFuncType = typeof(Func<>).Assembly
                .GetTypes().OfType<TypeInfo>()
                .Where(t => t.Name.StartsWith("Func`") && t.GenericTypeParameters.Count() == parameters.Count() + 1).First();

            // taskType Task<TResult>
            var returnType = method.ReturnType;

            var funcArguments = new List<Type>(parameterTypes);
            funcArguments.Add(returnType);
            var funcType = genericFuncType.MakeGenericType(funcArguments.ToArray());

            // delegate
            var dynamicMethodName = "_m" + Guid.NewGuid().ToString().Replace("-", "");
            var dynamicMethod = new DynamicMethod(dynamicMethodName, returnType, parameterTypes.ToArray(), Module, skipVisibility: true);

            // generate il
            var ilGenerator = dynamicMethod.GetILGenerator();

            // load parameters
            for (int i = 0; i < parameterTypes.Count(); i++)
            {
                ilGenerator.Emit(OpCodes.Ldarg, i);
                if (parameterTypes[i].IsValueType && parameterTypes[i] == typeof(object))
                {
                    ilGenerator.Emit(OpCodes.Unbox_Any, effectiveTypes[i]);
                }
                else if (effectiveTypes[i] != parameterTypes[i])
                {
                    ilGenerator.Emit(OpCodes.Castclass, effectiveTypes[i]);
                }
            }

            // call method
            ilGenerator.EmitCall(method.IsStatic ? OpCodes.Call : OpCodes.Callvirt, method, null);
            // cast result
            ilGenerator.Emit(OpCodes.Castclass, returnType);
            // return
            ilGenerator.Emit(OpCodes.Ret);

            var dynamicMethodDelegate = dynamicMethod.CreateDelegate(funcType);

            var mi = dynamicMethodDelegate.GetMethodInfo();
            var pi = mi.GetParameters();

            //var test = (Func<object, object, object, Task<int>>)dynamicMethodDelegate;
            //return test(parameters[0], parameters[1], parameters[2]);

            return dynamicMethodDelegate.DynamicInvoke(parameters.ToArray());
        }
    }
}
