using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using MassTransit;
using Microsoft.AspNetCore.Mvc;
using Microsoft.EntityFrameworkCore;
using SampleApp.Database;
using SampleApp.Database.Entities;
using SampleApp.MessageBus;
using StackExchange.Redis;

namespace SampleApp.Controllers
{
    /// <summary>
    /// demo controller
    /// </summary>
    [Route("api/[controller]")]
    [ApiController]
    public class ValuesController : ControllerBase
    {
        private readonly IBusControl _busControl;
        private readonly MyDbContext _myDbContext;
        private readonly ConnectionMultiplexer _connectionMultiplexer;

        public ValuesController(IBusControl busControl, MyDbContext myDbContext, ConnectionMultiplexer connectionMultiplexer)
        {
            _busControl = busControl;
            _myDbContext = myDbContext;
            _connectionMultiplexer = connectionMultiplexer;
        }

        /// <summary>
        /// executes an 'ok' sql query
        /// </summary>
        /// <returns></returns>
        [HttpGet]
        public async Task<ActionResult<IEnumerable<MyEntity>>> Get()
        {
            _myDbContext.Database.ExecuteSqlCommand("SELECT 1;");
            await _myDbContext.Database.ExecuteSqlCommandAsync("SELECT 2;");

            return await _myDbContext.MyEntities.Where(e => e.Id > 0).ToListAsync();
        }

        /// <summary>
        /// executes a bad sql query
        /// </summary>
        /// <returns></returns>
        [HttpGet("bad")]
        public ActionResult<IEnumerable<BadEntity>> Bad()   
        {
            try
            {
                return _myDbContext.BadEntities;
            }
            catch (Exception)
            {
                return Ok();
            }
        }

        /// <summary>
        /// publish a message with masstransit
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        [HttpGet("publish")]
        public async Task<ActionResult<string>> Publish(int id)
        {
            await _busControl.Publish(new MyMessage { Id = id });
            return Ok();
        }

        /// <summary>
        /// publish a message with redis
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        [HttpGet("publish-redis-sync")]
        public ActionResult<string> PublishRedisSync()
        {
            _connectionMultiplexer
                .GetDatabase()
                .Multiplexer
                .GetSubscriber()
                .Publish("channel1", "value1");

            return Ok();
        }

        /// <summary>
        /// publish a message with redis
        /// </summary>
        /// <param name="id"></param>
        /// <returns></returns>
        [HttpGet("publish-redis-async")]
        public async Task<ActionResult<string>> PublishRedisAsync()
        {
            await _connectionMultiplexer
                .GetDatabase()
                .Multiplexer
                .GetSubscriber()
                .PublishAsync("channel2", "value2");
            return Ok();
        }

        /// <summary>
        /// produces an http exception
        /// </summary>
        /// <returns></returns>
        [HttpGet("exception")]
        public ActionResult<string> Exception()
        {
            throw new NotImplementedException();
        }
    }
}
