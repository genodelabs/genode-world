/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <base/log.h>


#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

#include "greeter_server.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerCredentials;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;

// Logic and data behind the server's behavior.
class GreeterServiceImpl final : public Greeter::Service {
  Status SayHello(ServerContext* context, const HelloRequest* request,
                  HelloReply* reply) override {
		printf("say hello\n");
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    return Status::OK;
  }
};

std::string read_file(const char* filename)
{
	const long max_size = 4096;

	auto f = fopen(filename, "rb");

	if (f == nullptr) {
		Genode::error("cannot open file ", filename);
		throw -1;
	}

	std::string res;
	res.resize(max_size);

	// C++17 defines .data() which returns a non-const pointer
	const long size = fread(const_cast<char*>(res.data()), 1, max_size, f);
	res.resize(size);

	fclose(f);

	return res;
}

void RunServer() {
  GreeterServiceImpl service;

	auto certificate = read_file("/server.crt");
	auto private_key = read_file("/server.key");

	grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp = { private_key.c_str(), certificate.c_str() };
	grpc::SslServerCredentialsOptions ssl_opts;
	ssl_opts.pem_root_certs = "";
	ssl_opts.pem_key_cert_pairs.push_back(pkcp);
	std::shared_ptr<ServerCredentials> credentials = grpc::SslServerCredentials(ssl_opts);

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort("0.0.0.0:50051", credentials);
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on" << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

