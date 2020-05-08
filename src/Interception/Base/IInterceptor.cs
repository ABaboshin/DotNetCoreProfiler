namespace Interception.Base
{
    /// <summary>
    /// interceptor
    /// </summary>
    public interface IInterceptor
    {
        /// <summary>
        /// set this of intercepted class
        /// </summary>
        /// <param name="_this"></param>
        /// <returns></returns>
        object SetThis(object _this);

        /// <summary>
        /// add parameter of intercepted method
        /// </summary>
        /// <param name="value"></param>
        /// <returns></returns>
        object AddParameter(object value);

        /// <summary>
        /// set md token of intercepted method
        /// </summary>
        /// <param name="mdToken"></param>
        /// <returns></returns>
        object SetMdToken(int mdToken);

        /// <summary>
        /// set pointer to module version of intercepted method
        /// </summary>
        /// <param name="moduleVersionPtr"></param>
        /// <returns></returns>
        object SetModuleVersionPtr(long moduleVersionPtr);

        /// <summary>
        /// execute interceptor
        /// </summary>
        /// <returns></returns>
        object Execute();
    }
}
