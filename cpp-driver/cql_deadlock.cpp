#include <cassert>
#include <vector>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <cql/cql.hpp>
#include <cql/cql_error.hpp>
#include <cql/cql_event.hpp>
#include <cql/cql_connection.hpp>
#include <cql/cql_session.hpp>
#include <cql/cql_cluster.hpp>
#include <cql/cql_builder.hpp>
#include <cql/cql_execute.hpp>
#include <cql/cql_result.hpp>

// This function is called asynchronously every time an event is logged
void
log_callback(
    const cql::cql_short_t,
    const std::string& message)
{
    std::cout << "LOG: " << message << std::endl;
}

void 
removeFinishedFutures(std::vector<boost::shared_future< cql::cql_future_result_t> > &pendingFutures) {
  
    // The cleanup is not needed everytime
    if(pendingFutures.size() % 100 != 0) {
      return;
    }
    
    std::cout << std::endl << "Remove finished futures" << std::endl;
  
    // Are some futures finished?
    for(std::vector < boost::shared_future<cql::cql_future_result_t> >
      ::iterator iter = pendingFutures.begin(); 
      iter != pendingFutures.end(); 
      ) {
      
      boost::shared_future<cql::cql_future_result_t> future = *iter;
      
      // Remove finished futures
      if(future.is_ready()) {
        
        if(future.get().error.is_err()) {
          std::cerr << "Got error while executing future: " 
                << future.get().error.message << std::endl;
        }
        
        iter = pendingFutures.erase(iter);
      } else {
        ++iter;
      }
    }
}


void
demo(
    const std::string& host,
    bool               use_ssl)
{
    try
    {
            boost::shared_ptr<cql::cql_builder_t> builder = cql::cql_cluster_t::builder();
//		builder->with_log_callback(&log_callback);
            builder->add_contact_point(boost::asio::ip::address::from_string(host));

            if (use_ssl) {
                    builder->with_ssl();
            }

            boost::shared_ptr<cql::cql_cluster_t> cluster(builder->build());
            boost::shared_ptr<cql::cql_session_t> session(cluster->connect());

            std::vector<boost::shared_future< cql::cql_future_result_t> > pendingFutures;
                
            if (session) {
            
                    session -> set_keyspace("mykeyspace");

                    // write a query that create a table
                    boost::shared_ptr<cql::cql_query_t> create_table(
                       new cql::cql_query_t("CREATE TABLE IF NOT EXISTS table1 (key text PRIMARY KEY, value text);", cql::CQL_CONSISTENCY_ALL));

                    // send the query to Cassandra
                    boost::shared_future<cql::cql_future_result_t> future = session->query(create_table);

                    // wait for the query to execute
                    future.wait();

                    if(future.get().error.is_err()) {
                        std::cerr << "Unable to execute query: " << future.get().error.message << std::endl;
                        return;
                    }
                    
                    // now a small demonstration on the usage of prepared statements:
                    boost::shared_ptr<cql::cql_query_t> unbound_insert(
                        new cql::cql_query_t("INSERT INTO table1 (key, value) VALUES (?, ?);", cql::CQL_CONSISTENCY_ONE));

                    // compile the parametrized query on the server
                    future = session->prepare(unbound_insert);
                    future.wait();
                    std::cout << "prepare successful? " << (!future.get().error.is_err() ? "true" : "false") << std::endl;

                    // read the hash (ID) returned by Cassandra as identificator of prepared query
                    std::vector<cql::cql_byte_t> queryid = future.get().result->query_id();

                    size_t counter = 0;

                    while(true) {

                        boost::shared_ptr<cql::cql_execute_t> bound(
                           new cql::cql_execute_t(queryid, cql::CQL_CONSISTENCY_ALL));

                        // bind the query with concrete parameters
                        std::stringstream ss1;
                        std::stringstream ss2;
                        
                        ss1 << "row-" << counter;
                        bound->push_back(ss1.str());
                        
                        ss2 << "value-" << counter;
                        bound->push_back(ss2.str());

                        // send the concrete (bound) query
                        future = session->execute(bound);

                        // Add the future to the list of pending futures
                        pendingFutures.push_back(future);

			// Remove all finished futures
                        removeFinishedFutures(pendingFutures);

                        std::cout << "." << std::flush;
                        ++counter;
                    }

                    // close the connection session
                    session->close();
		}

		cluster->shutdown();
        std::cout << "THE END" << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

int
main(
    int    argc,
    char** vargs)
{
    bool ssl = false;
    std::string host;

    boost::program_options::options_description desc("Options");
    desc.add_options()
        ("help", "produce help message")
        ("ssl", boost::program_options::value<bool>(&ssl)->default_value(false), "use SSL")
        ("host", boost::program_options::value<std::string>(&host)->default_value("127.0.0.1"), "node to use as initial contact point");

    boost::program_options::variables_map variables_map;
    try {
        boost::program_options::store(boost::program_options::parse_command_line(argc, vargs, desc), variables_map);
        boost::program_options::notify(variables_map);
    }
    catch (boost::program_options::unknown_option ex) {
        std::cerr << desc << "\n";
        std::cerr << ex.what() << "\n";
        return 1;
    }

    if (variables_map.count("help")) {
        std::cerr << desc << "\n";
        return 0;
    }

    cql::cql_initialize();
    demo(host, ssl);

    cql::cql_terminate();
    return 0;
}
