using Interception.Attributes;
using Interception.Core.Extensions;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Threading.Tasks;

namespace Interception.Core
{
    public class ComposedInterceptor : IComposedInterceptor
    {
        private IMethodFinder _methodFinder = new MethodFinder();
        private List<IInterceptor> _childs = new List<IInterceptor>();
        private object[] _parameters;

        public object This { get; set; }
        public int MdToken { get; set; }
        public int TypeMdToken { get; set; }
        public int GenericTypeMdToken { get; set; }
        public long ModuleVersionPtr { get; set; }

        public void AddChild(IInterceptor interceptor)
        {
            //Console.WriteLine($"AddChild {interceptor.GetType().FullName} {_childs != null} this: {this != null}");
            _childs.Add(interceptor);
        }

        public void AddParameter(int num, object value)
        {
            //Console.WriteLine($"AddParameter {num} {value}");
            _parameters[num] = value;
        }

        public void SetArgumentCount(int number)
        {
            //Console.WriteLine($"SetArgumentNumber {number}");
            _parameters = new object[number];
        }

        public object GetParameter(int num)
        {
            return _parameters[num];
        }

        //private List<int> _genericTypeParameters = new List<int>();
        //public void AddGenericTypeParameter(int mdToken)
        //{
        //    Console.WriteLine($"AddGenericTypeParameter {mdToken}");
        //    _genericTypeParameters.Add(mdToken);
        //}

        private List<int> _genericMethodParameters = new List<int>();
        public void AddGenericMethodParameter(int mdToken)
        {
            Console.WriteLine($"AddGenericMethodParameter {mdToken}");
            _genericMethodParameters.Add(mdToken);
        }

        private MethodInfo FindMethod()
        {
            //if (ThisType is null)
            //{
            //    throw new Exception();
            //}

            //Console.WriteLine($"Thistype {ThisType != null}");

            //var mi = ThisType.Module.ResolveMethod(MdToken);
            //Console.WriteLine($"FindMethod {mi != null} {mi?.Name}");

            Type thisType = new TypeFinder().FindType(TypeMdToken, ModuleVersionPtr);
            Console.WriteLine($"TypeMdToken resolve {TypeMdToken} {thisType} x {This?.GetType()} x {thisType?.IsGenericType} {thisType?.GetGenericArguments().Length}");

            //if (thisType != null)
            //{
            //    foreach (var item in thisType.GetMethods(BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static | BindingFlags.NonPublic))
            //    {
            //        Console.WriteLine($"FindMethod thistype MdToken {MdToken} item {item.MetadataToken} {item.Name}");
            //    }



            //    //try
            //    //{
            //    //    foreach (var item in thisType.GetRuntimeMethods().Select(m => m.MetadataToken))
            //    //    {
            //    //        Console.WriteLine($"FindMethod MdToken {MdToken} item {item}");
            //    //    }
            //    //    var mi = thisType.Module.ResolveMethod(MdToken);
            //    //    Console.WriteLine($"method resolve {MdToken} {mi != null}");
            //    //}
            //    //catch (Exception ex)
            //    //{
            //    //    Console.WriteLine($"method resolve {ex}");
            //    //}
            //}

            //if (This != null) foreach (var item in This.GetType().GetMethods(BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static | BindingFlags.NonPublic))
            //    {
            //        Console.WriteLine($"FindMethod This.GetType() MdToken {MdToken} item {item.MetadataToken} {item.Name}");
            //    }

            //foreach (var item in _genericTypeParameters)
            //{
            //    Console.WriteLine($"generic {item} {new TypeFinder().FindType(item, ModuleVersionPtr)} x");
            //}

            Console.WriteLine($"FindMethod {This} {MdToken} {ModuleVersionPtr} {_parameters.Length}");
            return (MethodInfo)_methodFinder.FindMethod(MdToken, ModuleVersionPtr, thisType?.GetGenericArguments());
        }

        public object Execute()
        {
            Validate();
            var method = FindMethod();
            var isAsync = method.IsReturnTypeTask();

            Console.WriteLine($"Execute {method.Name}");

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

        #region internal

        private void ExecuteBefore()
        {
            foreach (var item in _childs)
            {
                item.This = This;
                item.MdToken = MdToken;
                item.ModuleVersionPtr = ModuleVersionPtr;
                item.SetParameters(_parameters);
                item.ExecuteBefore();
                for (int i = 0; i < _parameters.Length; i++)
                {
                    if (item.IsParameterModified(i))
                    {
                        _parameters[i] = item.GetParameter(i);
                    }
                }
            }
        }

        private void ExecuteAfter()
        {
            foreach (var item in _childs)
            {
                item.SetParameters(_parameters);
                item.Exception = _exception;
                item.Result = _result;
                item.ExecuteAfter();
            }
        }

        private object _result = null;
        private Exception _exception = null;
        //private Type thisType;

        private bool SkipExecution()
        {
            var skip = false;
            foreach (var item in _childs)
            {
                if (item.SkipExecution())
                {
                    skip = true;
                    _result = item.Result;
                }
            }

            return skip;
        }

        private object ExecuteSyncInternal()
        {
            var method = FindMethod();

            try
            {
                ExecuteBefore();
                if (!SkipExecution())
                {
                    _result = method.Invoke(This, _parameters);
                }

                ExecuteAfter();
                return _result;
            }
            catch (Exception ex)
            {
                _exception = ex;
                ExecuteAfter();
                throw;
            }
        }

        private async Task ExecuteAsyncInternal()
        {
            var method = FindMethod();

            try
            {
                ExecuteBefore();
                if (!SkipExecution())
                {
                    var result = (Task)method.Invoke(This, _parameters);
                    await result;
                    _result = result;
                }

                ExecuteAfter();
            }
            catch (Exception ex)
            {
                _exception = ex;
                ExecuteAfter();
                throw;
            }
        }

        private async Task<T> ExecuteWithMetrics<T>(Delegate funcDelegate)
        {
            try
            {
                ExecuteBefore();
                if (!SkipExecution())
                {
                    var func = (Func<Task<T>>)funcDelegate;
                    _result = await func.Invoke();
                }

                ExecuteAfter();
                return (T)_result;
            }
            catch (Exception ex)
            {
                _exception = ex;
                ExecuteAfter();
                throw;
            }
        }

        internal static readonly ModuleBuilder Module;

        static ComposedInterceptor()
        {
            var asm = AssemblyBuilder.DefineDynamicAssembly(new AssemblyName("Interception"), AssemblyBuilderAccess.Run);
            Module = asm.DefineDynamicModule("DynamicModule");
        }

        private object ExecuteAsyncWithResultInternal()
        {
            var method = FindMethod();

            var underilyingType = ((TypeInfo)method.ReturnType).GenericTypeArguments[0];

            var createExecutionDelegateGeneric = typeof(IComposedInterceptor).GetMethods(BindingFlags.Instance | BindingFlags.NonPublic).Where(m => m.Name == nameof(CreateExecutionDelegate)).First();
            var createExecutionDelegate = createExecutionDelegateGeneric.MakeGenericMethod(underilyingType);
            var executionDelegate = createExecutionDelegate.Invoke(this, new object[] { });

            var executeWithMetricsGeneric = typeof(IComposedInterceptor).GetMethods(BindingFlags.Instance | BindingFlags.NonPublic).Where(m => m.Name == nameof(ExecuteWithMetrics)).First();
            var executeWithMetrics = executeWithMetricsGeneric.MakeGenericMethod(underilyingType);

            return executeWithMetrics.Invoke(this, new object[] { executionDelegate });
        }

        private Delegate CreateExecutionDelegate<T>()
        {
            var method = FindMethod();

            Func<Task<T>> func = () =>
            {
                return (Task<T>)method.Invoke(This, _parameters);
            };

            return func;
        }

        protected void Validate()
        {
            var method = FindMethod();
            foreach (var p in method.GetParameters())
            {
                var validationAttributes = p.GetCustomAttributes<ParameterValidationAttribute>();

                foreach (var validationAttribute in validationAttributes)
                {
                    validationAttribute.Validate(_parameters[p.Position]);
                }
            }
        }
        #endregion
    }
}
