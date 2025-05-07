#include <iostream>
#include <string>
#include <curl/curl.h>

// 回调函数，用于接收 CURLOPT_WRITEFUNCTION 的数据
// data: 指向接收到数据的指针
// size: 每个数据块的大小 (通常是 1)
// nmemb: 数据块的数量
// userdata: 我们通过 CURLOPT_WRITEDATA 传递的用户数据 (在这里是一个 stringstream 的指针)
size_t WriteCallback(void* data, size_t size, size_t nmemb, void* userdata)
{
  size_t total_size = size * nmemb;
  std::string* response_body = reinterpret_cast<std::string*>(userdata);
  if (response_body)
  {
    response_body->append(static_cast<char*>(data), total_size);
  }
  return total_size; // 返回实际处理的字节数
}

int main()
{
  CURL* curl;
  CURLcode res;
  std::string response_body; // 用于存储响应体

  // 1. 初始化 libcurl 环境
  // 必须在使用任何其他 libcurl 函数之前调用
  res = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (res != CURLE_OK)
  {
    std::cerr << "curl_global_init() failed: " << curl_easy_strerror(res) << std::endl;
    return 1;
  }

  // 2. 创建一个 easy handle
  curl = curl_easy_init();
  if (!curl)
  {
    std::cerr << "curl_easy_init() failed" << std::endl;
    curl_global_cleanup(); // 初始化失败也要清理
    return 1;
  }

  // 3. 设置请求的 URL (HTTPS 地址)
  // 可以替换成任何有效的 HTTPS URL
  curl_easy_setopt(curl, CURLOPT_URL, "https://kekxv.com");

  // 4. 设置写入数据的回调函数和用户数据
  // libcurl 会将接收到的响应体数据传递给 WriteCallback 函数
  // WriteCallback 函数会将数据追加到 response_body 字符串中
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);

  // --- HTTPS 相关设置 ---

  // IMPORTANT: 禁用证书验证！
  // 在生产环境中，请务必启用并正确配置证书验证。
  // 启用验证并指定 CA 证书：
  // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
  // curl_easy_setopt(curl, CURLOPT_CAINFO, "/path/to/your/cacert.pem");
  // 或者让 libcurl 使用系统的 CA 证书库：
  // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
  // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L); // 验证主机名匹配
  // 对于本 demo，我们禁用验证以便于运行：
  std::cerr <<
    "Warning: Certificate verification disabled for demo purposes. DO NOT use this in production without enabling verification."
    << std::endl;
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // 不验证对端证书
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); // 不验证主机名 (通常和 VERIFYPEER 一起设置)


  // 5. 执行请求
  res = curl_easy_perform(curl);

  // 检查请求是否成功
  if (res != CURLE_OK)
  {
    std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
  }
  else
  {
    // 6. 获取并打印响应状态码
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    std::cout << "HTTP Status Code: " << http_code << std::endl;

    // 打印响应体
    std::cout << "Response Body:" << std::endl;
    // 为了防止输出过大，这里只打印前 N 个字符
    const size_t MAX_PRINT_SIZE = 500;
    if (response_body.length() > MAX_PRINT_SIZE)
    {
      std::cout << response_body.substr(0, MAX_PRINT_SIZE) << "..." << std::endl;
    }
    else
    {
      std::cout << response_body << std::endl;
    }
  }

  // 7. 清理 easy handle
  curl_easy_cleanup(curl);

  // 清理 libcurl 环境
  curl_global_cleanup();

  return 0;
}
