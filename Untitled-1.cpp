#include "server.h"
#include <thread>
#include <cstdlib>
#include <ctime>

using namespace std;
using namespace webserver;

void downloadTrack(json track);
uint16_t evtPort = 1349;

uint16_t pickPort() {
  string output = exec("netstat -ltn");
  output.erase(0, output.find("\n") + 1);
  output.erase(0, output.find("\n") + 1);
  vector<uint16_t> usedPorts;
  for (; output.substr(0, 4) == "tcp "; output.erase(0, output.find("\n") + 1)) {
    usedPorts.push_back(stoi(output.substr(output.find(":") + 1)));
  }
  bool used = true;
  srand(time(0));
  uint16_t port;
  while (used) {
    port = rand() % ((9999 - 1025) + 1) + 1025;
    used = false;
    for (auto i : usedPorts) {
      if (i == port) {
        used = true;
        break;
      }
    }
  }
  return port;
}
void clearPort(uint16_t port) {}
bool fileExists(const string& name) {
  return access(name.c_str(), F_OK) != -1;
}
Response all() {
  string url = request.base_url;
  string last = url.substr(url.rfind("/") + 1);
  if (last.find(".") == last.npos) {
    if (last.length() > 0) {
      url += "/";
    }
    url += "index.html";
  }
  if (fileExists("." + url)) {
    return send_file("." + url);
  } else {
    throw 404;
  }
}
Response api() {
  Response response;
  if (request.base_url == "/api/pid") {
    response.text = to_string(getppid());
    response.status = 200;
    response.statusText = "OK";
    response.headers.set("Content-Type", "application/json");
  } else if (request.base_url == "/api/echo") {
    response.vector = request.bodyVector;
    response.usingText = false;
    response.usingVector = true;
    response.headers.set("Content-Type", request.headers.get("Content-Type"));
    response.length = request.bodyLength;
  } else if (request.base_url == "/api/401") {
    throw 401;
  } else if (request.base_url == "/api/createArtistRadio") {
    string fileContent, line, artistId;
    if (request.method == "POST") {
      artistId = reinterpret_cast<char*>(request.bodyVector.data());
      response.headers.set("Connection", "keep-alive");
    } else {
      artistId = request.args.get("artistId");
      response.headers.set("Connection", "close");
    }
    ifstream file;
    file.open("./playlist/headers.yml");
    while (getline(file, line)) {
      fileContent += line + "\n";
    }
    file.close();
    ofstream outputFile ("./playlist/headers.yml");
    regex re ("Content-Length: [0-9]+");
    fileContent = regex_replace(fileContent, re, "Content-Length: " + to_string(artistId.length() + 23));
    re = regex("Content-Type: application/.*\n");
    fileContent = regex_replace(fileContent, re, "Content-Type: application/x-www-form-urlencoded\n");
    outputFile << fileContent;
    outputFile.close();
    smatch match;
    re = regex("X-User-Id: [0-9]+");
    regex_search(fileContent, match, re);
    re = regex("[0-9]+");
    line = match.str(0);
    regex_search(line, match, re);
    line = match.str(0);
    string cmd = "curl -X POST https://us.api.iheart.com/api/v2/playlists/" + line + "/ARTIST/" + artistId + " "
                 "-H \"@/home/aidyn/webapp/templates/icons/playlist/headers.yml\" "
                 "-d \"contentId=" + artistId + "&playedFrom=0\"";
    response.text = exec(cmd);
    response.headers.set("Content-Type", "application/json; charset=utf-8");
    return response;
  } else if (request.base_url == "/api/command") {
    if (request.method != "POST") {
      throw 405;
    }
    Response response;
    response.text = exec(*new string(reinterpret_cast<char*>(request.bodyVector.data()), request.bodyLength));
    response.headers.set("Content-Type", "text/plain");
    return response;
    // g++ ./Untitled-1.cpp -o ./Untitled-1 && ./Untitled-1
    // https://us.api.iheart.com/api/v3/search/combined?boostMarketId=101&bundle=true&keyword=true&keywords=they might be giants&countryCode=US&artist=false&playlist=false&station=false&podcast=false&track=true
  } else if (request.base_url == "/api/downloadPlaylist") {
    ifstream file;
    file.open("./playlist/content.json");
    string fileContent, line;
    getline(file, fileContent);
    file.close();
    json content = json::parse(fileContent);
    fileContent = "";
    bool event;
    if (request.method == "POST") {
      string playlistId (reinterpret_cast<char*>(request.bodyVector.data()), request.bodyLength);
      content["stationId"] = playlistId.substr(0, playlistId.find("\n"));
      event = playlistId.substr(playlistId.find("\n") + 1) == "true";
    } else if (request.args.get("playlistId") == "") {
      response.text = "{\n"
                      "  \"error\": {\n"
                      "    \"code\": 400,\n"
                      "    \"statusText\": \"Bad Request\",\n"
                      "    \"description\": \"Missing URL parameter 'playlistId'\"\n"
                      "  }\n"
                      "}";
      response.status = 400;
      response.statusText = "Bad Request";
      response.headers.set("Content-Type", "application/json");
      return response;
    } else {
      content["stationId"] = request.args.get("playlistId");
      event = request.args.get("event") == "true";
    }
    content["stationId"] = findAndReplace("-", "::", content["stationId"]);
    file.open("./playlist/headers.yml");
    while (getline(file, line)) {
      fileContent += line + "\n";
    }
    file.close();
    ofstream outputFile ("./playlist/headers.yml");
    regex re ("Content-Length: [0-9]+");
    fileContent = regex_replace(fileContent, re, "Content-Length: " + to_string(content.dump().length()));
    re = regex("Content-Type: application/.*\n");
    outputFile << regex_replace(fileContent, re, "Content-Type: application/json\n");
    outputFile.close();
    outputFile.open("./playlist/content.json");
    outputFile << content.dump();
    outputFile.close();
    string cmd = "curl -X POST https://us.api.iheart.com/api/v2/playback/streams "
                 "-H \"@/home/aidyn/webapp/templates/icons/playlist/headers.yml\" "
                 "--data-binary \"@/home/aidyn/webapp/templates/icons/playlist/content.json\" "
                 "-D /tmp/headers.txt";
    response.text = exec(cmd);
    if (event) {
      evtPort = pickPort();
      response.headers.set("X-Download-Id", to_string(evtPort));
    }
    if (request.args.get("prettyPrint") == "true") {
      json xy = json::parse(response.text);
      response.text = xy.dump(2);
    }
    response.headers.set("Content-Type", "application/json");
    response.status = 200;
    response.statusText = "OK";
    response.usingText = true;
    response.usingVector = false;
    send_response(response, server.clientSocket);
    try {
    close(server.clientSocket);
    cout << 1 << endl;
    json tracks = json::parse(response.text)["items"];
    int length = tracks.size();
    thread threads[length];
    cout << 1 << endl;
    for (int i = 0; i < length; i++) {
      threads[i] = thread(downloadTrack, tracks[i]);
    }
    for (int i = 0; i < length; i++) {
      threads[i].join();
    }
    cout << 1 << endl;
    exit(0);
    } catch(...) {
      cout << "error" << endl;
      exit(1);
    }
  } else if (request.base_url == "/api/download_m3u8") {
    if (request.method != "POST") {
      throw 405;
    }
    string body = reinterpret_cast<char*>(request.bodyVector.data());
    response.text = exec("curl https://api.iheart.com/api/v3/catalog/tracks/" + body.substr(body.find("\n") + 1, body.rfind("\n")));
    json track = json::parse(response.text)["tracks"][0];
    for (const char* i : { "title", "artistName", "albumName", "id" }) {
      track["content"][i] = track[i];
    }
    track["content"]["imagePath"] = track["imageUrl"];
    track["streamUrl"] = body.substr(0, body.find("\n"));
    if (body.substr(body.rfind("\n") + 1) == "true") {
      evtPort = pickPort();
      response.headers.set("X-Download-Id", to_string(evtPort));
    }
    response.headers.set("Content-Type", "application/json");
    send_response(response, server.clientSocket);
    close(server.clientSocket);
    downloadTrack(track);
    exit(0);
  } else if (request.base_url == "/api/trackDownload.cgi") {
    uint16_t port = stoi(request.args.get("downloadId"));
    int serverId = socket(AF_INET, SOCK_STREAM, 0), clientId;
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind(serverId, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    int len = stoi(request.args.get("len"));
    listen(serverId, len);
    string msg;
    char buffer[11] = {0};
    send(server.clientSocket, "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nServer: kitten/0.1\r\n\r\n: i\n\n", 77, 0);
    for (int i = 0; i < len; i++) {
      msg = "data: ";
      clientId = accept(serverId, nullptr, nullptr);
      recv(clientId, buffer, sizeof(buffer), 0);
      buffer[10] = 0;
      cout << buffer << endl;
      msg += buffer;
      msg += "\n\n";
      send(server.clientSocket, msg.c_str(), msg.length(), 0);
      memset(buffer, 0, sizeof(buffer));
    }
    close(serverId);
    clearPort(port);
    exit(0);
  } else if (request.base_url == "/api/meta.cgi") {
    throw 500;
    ifstream file("./meta.json");
    string msg = "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nConnection: keep-alive\r\nContent-Length: ", content;
    getline(file, content);
    content = "data: " + content + "\n\n";
    msg += to_string(content.length()) + "\r\n\r\n" + content;
    send(server.clientSocket, msg.c_str(), msg.length(), 0);
    pid_t c_pid = fork();
    while (true) {
      if (c_pid == 0) {
        system("sleep 15");
        send(server.clientSocket, ": comment", 9, 0);
      } else {
        system("inotifywait --fromfile ~/webapp/templates/icons/meta.json -e modify");
        file = ifstream("./meta.json");
        getline(file, msg);
        msg = "data: " + msg + "\n\n";
        send(server.clientSocket, msg.c_str(), msg.length(), 0);
      }
    }
  } else {
    response.text = "{\n"
                    "  \"status\": 404,\n"
                    "  \"statusText\": \"Not Found\"\n"
                    "}";
    response.status = 404;
    response.statusText = "Not Found";
    response.headers.set("Content-Type", "application/json"); 
  }
  return response;
}
void downloadTrack(json track) {
  ofstream src ("./tracks.bin", std::ios::app | std::ios::binary);
  int __id__ = track["content"]["id"];
  cout << __id__ << " " << sizeof(int) << endl;
  src.write(reinterpret_cast<const char*>(&__id__), sizeof(int));
  cout << 2 << endl;
  string url = track["streamUrl"];
  src.write(url.c_str(), url.length());
  cout << 3 << endl;
  src.close();
  if (track["only_src"] == true) {
    return;
  }
  if (track["content"]["artistName"] == "Goo Goo Dolls") {
    track["content"]["artistName"] = "The Goo Goo Dolls";
  }
  regex re ("#.*\n");
  string m3u8 = regex_replace(exec("curl " + url), re, ""), start = url.substr(0, url.rfind("/")  + 1);
  string id = to_string(track["content"]["id"]), line = m3u8;
  int occs = 0;
  while (line.find("\n") != line.npos) {
    occs++;
    line.erase(0, line.find("\n") + 1);
  }
  ostringstream stream;
  for (int i = 0; i < occs; i++) {
    stream.str("");
    stream << "curl " << start << m3u8.substr(0, m3u8.find("\n")) << 
    " -o /tmp/" << id << "_" << i << ".aac";
    system(stream.str().c_str());
    m3u8.erase(0, m3u8.find("\n") + 1);
  }
  stream.str("");
  stream << "curl -D /tmp/" << id << ".txt \"" << *new string(track["content"]["imagePath"]) << "?ops=fit(1400,1400)\" -o /tmp/" << id << ".jpg";
  system(stream.str().c_str());
  stream.str("");
  stream << "ffmpeg -i \"concat:";
  for (int i = 0; i < occs; i++) {
    stream << "/tmp/" << id << "_" << i << ".aac";
    if (i < occs - 1) {
      stream << "|";
    } else {
      ifstream headers("/tmp/" + id + ".txt");
      string line;
      while (getline(headers, line)) {
        for (int i = 0; i < line.length(); i++) {
          if (65 <= line[i] && line[i] <= 90) {
            line[i] += 32;
          }
        }
        if (line.find("content-type: image/") != line.npos || line == "content-type: binary/octet-stream\r") {
          stream << "\" -i /tmp/" << id << ".jpg" <<
          " -map 0:0 -map 1:0 -c:a mp3 -id3v2_version 3 -metadata:s:v" << 
          " title=\"Album cover\" -metadata:s:v comment=\"Cover (front)";
          break;
        }
      }
      headers.close();
      stream << 
      "\" -metadata title=" << track["content"]["title"] <<
      " -metadata artist=" << track["content"]["artistName"] <<
      " -metadata album=" << track["content"]["albumName"] <<
      " -n /home/aidyn/webapp/templates/icons/" << id << ".mp3";
    }
  }
  system(stream.str().c_str());
  int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(evtPort);
  serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
  connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
  send(clientSocket, id.c_str(), id.length(), 0);
  close(clientSocket);
  stream.str("");
  stream << "rm ";
  for (int i = 0; i < occs; i++) {
    stream << "/tmp/" << id << "_" << i << ".aac ";
  }
  stream << "/tmp/" << id << ".jpg /tmp/" << id << ".txt";
  system(stream.str().c_str());
}
Response favicon() {
  Response response;
  response.text = "Coming soon";
  response.status = 404;
  response.statusText = "Not Found";
  response.headers.set("Content-Type", "text/plain");
  return response;
}
Response err(int status) {
  Response response = defaultErr(status);
  string ps = response.text.substr(33, response.text.length() - 1);
  response.text.erase(33, response.text.length() - 1);
  string line;
  ifstream file("./err.html");
  while (getline(file, line)) {
    response.text += line + "\n";
  }
  response.text += ps;
  return response;
}
int main(int argc, char** argv) {
  if (argc > 1) {
    json track;
    track["content"]["id"] = 653171;
    track["streamUrl"] = "http://127.0.0.1:8080";
    track["only_src"] = true;
    downloadTrack(track);
    exit(0);
  }
  cout << 4 << endl;
  srand(time(0));
  server.addRequestEvent("/favicon\\.ico", favicon);
  server.addRequestEvent("/api/.*", api);
  server.addRequestEvent(".*", all);
  server.setErrorHandler(err);
  server.run(1349);
}
