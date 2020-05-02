using Interception.Base.Extensions;
using Interception.Tracing.Extensions;
using OpenTracing;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Threading.Tasks;

namespace Interception.Base
{
    public abstract class BaseInterceptor
    {
        protected IMethodFinder _methodFinder = new MethodFinder();

        protected List<object> _parameters = new List<object>();

        protected object _this;

        protected int _mdToken;

        protected long _moduleVersionPtr;

        public object SetThis(object _this)
        {
            Console.WriteLine($"SetThis {_this}");
            this._this = _this;
            return this;
        }

        public object AddParameter(object value)
        {
            Console.WriteLine($"AddParameter {value}");
            _parameters.Add(value);
            return this;
        }

        public object SetMdToken(int mdToken)
        {
            Console.WriteLine($"SetMdToken {mdToken}");
            _mdToken = mdToken;
            return this;
        }

        public object SetModuleVersionPtr(long moduleVersionPtr)
        {
            Console.WriteLine($"SetModuleVersionPtr {moduleVersionPtr}");
            _moduleVersionPtr = moduleVersionPtr;
            return this;
        }

        public abstract object Execute();

        protected virtual void EnrichAfterExecution(object result, IScope scope)
        {
        }

        protected virtual IScope CreateScope()
        {
            throw new NotImplementedException();
        }

        protected virtual MethodInfo FindMethod()
        { 
            return (MethodInfo)_methodFinder.FindMethod(_mdToken, _moduleVersionPtr);
        }

        protected object ExecuteInternal(bool metricsEnabled)
        {
            var method = FindMethod();
            var isAsync = method.IsReturnTypeTask();

            if (!metricsEnabled)
            {
                return method.Invoke(_this, _parameters.ToArray());
            }

            if (!isAsync)
            {
                return ExecuteSyncInternal();
            }
            else if (!method.IsReturnTypeTaskWithResult())
            {
                return ExecuteAsyncInternal();
            }
            else
            {
                return ExecuteAsyncWithResultInternal();
            }
        }

        protected object ExecuteSyncInternal()
        {
            var method = FindMethod();

            using (var scope = CreateScope())
            {
                try
                {
                    var result = method.Invoke(_this, _parameters.ToArray());
                    EnrichAfterExecution(result, scope);
                    return result;
                }
                catch (Exception ex)
                {
                    scope.Span.SetException(ex);
                    throw;
                }
            }
        }

        protected async Task ExecuteAsyncInternal()
        {
            Console.WriteLine("ExecuteAsyncInternal");
            var method = FindMethod();

            using (var scope = CreateScope())
            {
                try
                {
                    await (Task)method.Invoke(_this, _parameters.ToArray());
                }
                catch (Exception ex)
                {
                    scope.Span.SetException(ex);
                    throw;
                }
            }
        }

        internal static readonly ModuleBuilder Module;

        static BaseInterceptor()
        {
            var asm = AssemblyBuilder.DefineDynamicAssembly(new AssemblyName("Interception"), AssemblyBuilderAccess.Run);
            Module = asm.DefineDynamicModule("DynamicModule");
        }

        protected object ExecuteAsyncWithResultInternal()
        {
            // arg count: parameters count + 1 if is instance method
            var parameters = new List<object>();
            if (_this != null)
            {
                parameters.Add(_this);
            }

            parameters.AddRange(_parameters);

            Console.WriteLine($"parameters {parameters.Count()}");
            // resolve Func<T1, ... Task<TResult>>
            var genericFuncType = typeof(Func<>).Assembly
                .GetTypes().OfType<TypeInfo>()
                .Where(t => t.Name.StartsWith("Func`") && t.GenericTypeParameters.Count() == parameters.Count() + 1).First();

            // taskType Task<TResult>
            var method = FindMethod();
            var returnType = method.ReturnType;

            var parameterTypes = parameters.Select(p => p.GetType()).ToList();
            var funcArguments = parameterTypes;
            funcArguments.Add(returnType);
            var funcType = genericFuncType.MakeGenericType(funcArguments.ToArray());

            // delegate
            var dynamicMethodName = Guid.NewGuid().ToString().Replace("-", "");
            var dynamicMethod = new DynamicMethod(dynamicMethodName, returnType, parameterTypes.ToArray(), Module, skipVisibility: true);

            // generate il
            var ilGenerator = dynamicMethod.GetILGenerator();

            // load parameters
            for (int i = 0; i < parameterTypes.Count(); i++)
            {
                ilGenerator.Emit(OpCodes.Ldarg, i);
                if (parameterTypes[i].IsValueType)
                {
                    ilGenerator.Emit(OpCodes.Unbox_Any, parameterTypes[i]);
                }
                else
                {
                    ilGenerator.Emit(OpCodes.Castclass, parameterTypes[i]);
                }
            }

            // call method
            ilGenerator.Emit(method.IsStatic ? OpCodes.Call : OpCodes.Callvirt, method);
            // cast result
            ilGenerator.Emit(OpCodes.Castclass, returnType);
            // return
            ilGenerator.Emit(OpCodes.Ret);

            var dynamicMethodDelegate = dynamicMethod.CreateDelegate(funcType);

            return dynamicMethodDelegate.DynamicInvoke(parameters);
        }
    }
}
