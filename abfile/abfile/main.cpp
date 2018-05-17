// abfile.cpp : Definiert den Einstiegspunkt f�r die Konsolenanwendung.
//

#include "stdafx.h"
#include <sstream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "SimpleConfigManager.h"

using namespace std;
using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;

int main()
{
    HttpsServer server("server.crt", "server.key");
    server.config.port = 8081;

    server.resource["^/info$"]["GET"] =
        [](shared_ptr<HttpsServer::Response> response, shared_ptr<HttpsServer::Request> request)
    {
        stringstream stream;
        stream << "<h1>Request from " << request->remote_endpoint_address() << ":" << request->remote_endpoint_port() << "</h1>";

        stream << request->method << " " << request->path << " HTTP/" << request->http_version;

        stream << "<h2>Query Fields</h2>";
        auto query_fields = request->parse_query_string();
        for (auto &field : query_fields)
            stream << field.first << ": " << field.second << "<br>";

        stream << "<h2>Header Fields</h2>";
        for (auto &field : request->header)
            stream << field.first << ": " << field.second << "<br>";

        response->write(stream);
    };

    // Default GET-example. If no other matches, this anonymous function will be called.
    // Will respond with content in the web/-directory, and its subdirectories.
    // Default file: index.html
    // Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
    server.default_resource["GET"] =
        [](shared_ptr<HttpsServer::Response> response, shared_ptr<HttpsServer::Request> request)
    {
        try
        {
            auto web_root_path = boost::filesystem::canonical("web");
            auto path = boost::filesystem::canonical(web_root_path / request->path);
            // Check if path is within web_root_path
            if (distance(web_root_path.begin(), web_root_path.end()) > distance(path.begin(), path.end()) ||
                !equal(web_root_path.begin(), web_root_path.end(), path.begin()))
                throw invalid_argument("path must be within root path");
            if (boost::filesystem::is_directory(path))
                path /= "index.html";

            SimpleWeb::CaseInsensitiveMultimap header;

            // Uncomment the following line to enable Cache-Control
            // header.emplace("Cache-Control", "max-age=86400");

            auto ifs = make_shared<ifstream>();
            ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);

            if (*ifs)
            {
                auto length = ifs->tellg();
                ifs->seekg(0, ios::beg);

                header.emplace("Content-Length", to_string(length));
                response->write(header);

                // Trick to define a recursive function within this scope (for example purposes)
                class FileServer
                {
                public:
                    static void read_and_send(const shared_ptr<HttpsServer::Response> &response, const shared_ptr<ifstream> &ifs)
                    {
                        // Read and send 128 KB at a time
                        vector<char> buffer(131072);
                        streamsize read_length;
                        if ((read_length = ifs->read(&buffer[0], static_cast<streamsize>(buffer.size())).gcount()) > 0)
                        {
                            response->write(&buffer[0], read_length);
                            if (read_length == static_cast<streamsize>(buffer.size()))
                            {
                                response->send([response, ifs](const SimpleWeb::error_code &ec)
                                {
                                    if (!ec)
                                        read_and_send(response, ifs);
                                    else
                                        cerr << "Connection interrupted" << endl;
                                });
                            }
                        }
                    }
                };
                FileServer::read_and_send(response, ifs);
            }
            else
                throw invalid_argument("could not read file");
        }
        catch (const exception &e)
        {
            response->write(SimpleWeb::StatusCode::client_error_bad_request, "Could not open path " + request->path + ": " + e.what());
        }
    };

    server.on_error =
        [](shared_ptr<HttpsServer::Request> /*request*/, const SimpleWeb::error_code & /*ec*/)
    {
        // Handle errors here
        // Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
    };

    // Start server
    server.config.thread_pool_size = 4;
    server.start();

    return 0;
}

#pragma warning(disable: 4503)
