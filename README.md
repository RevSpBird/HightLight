# HightLight
a plugin for ida of version 7.2 to help know F5 window codes better
Note:
           ida 在7.x版本中由于主程序都变为64位程序，所以要求插件也变为64位dll，因此在vs工程需要以下修改：
           主要列出比较重要的点和对比32位插件vs设置的变化
           1：在对应的包含目录添加sdk include目录(需要注意的是如果要对F5伪代码窗口进行操作，需要将ida程序plugin目录中hexrays.hpp防到目录下)
           2：预处理器同样是添加__NT__;__IDP__（编译64位版本需多加宏 __EA64__ ）
           3：附加库目录对应要修改为lib\x64_win_vc_32（这个是编写处理32位程序的lib库，若要编写64位的那么对应变为 x64_win_vc_64）
           4：dll的名称有要求，由以前的.p64和.plw 对应64位和32位变更为 xxx64.dll 和 xxx.dll
           
           其中最主要的是ida.lib的路径，若选错，会报fatal error错误
