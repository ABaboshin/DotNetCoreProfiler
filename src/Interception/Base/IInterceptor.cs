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
        void SetThis(object _this);

        /// <summary>
        /// set argument number
        /// </summary>
        /// <param name="number"></param>
        /// <returns></returns>
        void SetArgumentNumber(int number);

        /// <summary>
        /// add parameter of intercepted method
        /// </summary>
        /// <param name="num"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        void AddParameter(int num, object value);

        /// <summary>
        /// get paraemeter value
        /// </summary>
        /// <param name="num"></param>
        /// <returns></returns>
        object GetParameter(int num);

        /// <summary>
        /// set md token of intercepted method
        /// </summary>
        /// <param name="mdToken"></param>
        /// <returns></returns>
        void SetMdToken(int mdToken);

        /// <summary>
        /// set pointer to module version of intercepted method
        /// </summary>
        /// <param name="moduleVersionPtr"></param>
        /// <returns></returns>
        void SetModuleVersionPtr(long moduleVersionPtr);

        /// <summary>
        /// execute interceptor
        /// </summary>
        /// <returns></returns>
        object Execute();
    }
}
