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
            scope.Span.SetTag("result", result?.ToString());
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
            var method = FindMethod();

            var underilyingType = ((TypeInfo)method.ReturnType).GenericTypeArguments[0];

            var createExecutionDelegateGeneric = typeof(BaseInterceptor).GetMethods(BindingFlags.Instance | BindingFlags.NonPublic).Where(m => m.Name == nameof(CreateExecutionDelegate)).First();
            var createExecutionDelegate = createExecutionDelegateGeneric.MakeGenericMethod(underilyingType);
            var executionDelegate = createExecutionDelegate.Invoke(this, new object[] { });

            var executeWithMetricsGeneric = typeof(BaseInterceptor).GetMethods(BindingFlags.Instance | BindingFlags.NonPublic).Where(m => m.Name == nameof(ExecuteWithMetrics)).First();
            var executeWithMetrics = executeWithMetricsGeneric.MakeGenericMethod(underilyingType);

            return executeWithMetrics.Invoke(this, new object[] { executionDelegate });
        }

        private async Task<T> ExecuteWithMetrics<T>(Delegate funcDelegate)
        {
            using (var scope = CreateScope())
            {
                try
                {
                    var func = (Func<Task<T>>)funcDelegate;

                    var result = await func.Invoke();

                    Console.WriteLine($"result {result} {result.GetType().Name}");

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

        private Delegate CreateExecutionDelegate<T>()
        {
            var method = FindMethod();

            Func<Task<T>> func = () => {
                return (Task<T>)method.Invoke(_this, _parameters.ToArray());
            };

            return func;
        }
    }
}
