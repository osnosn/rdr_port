# [rdr_port](https://github.com/osnosn/rdr_port/)
Temporary redirect port in N seconds, use iptables or nftable rule.   
临时重定向端口， 使用 ipt 或 nft 规则。  

In the two directories are the programs (written in C) corresponding to the IPT and NFT rules.  
just execute `gcc -O -o xxxx rdr_port_ipt` to complite an executable file.  
`strip xxxx` can make the executable file smaller.  

两个目录中分别是对应 IPT 和 NFT 规则的 C语言程序。  
只需执行 `gcc -O -o xxxx  rdr_port_ipt` 即可生成可执行文件。  
`strip xxxx` 可以让可执行文件更小一点。  

--------
Execute `rdr_port_ipt` or  `rdr_port_nft` :
* Insert a port redirection rule into the iptables rule table or nftable chain according to the source IP provided.
* The program resides in the background and waits for N seconds (depending on the command line parameters).
* Remove the previously inserted redirection rule from the iptables or nftable.
* The program done & exit.

The compiled executable file can be set with suid (chmod 4555 ...), which is called and executed by web script and temporarily redirected the port.   
For example 443 -> 22, You can use the SSH client connect to port 443 in a few seconds, and then even if the redirection rule is deleted, your SSH connection will not be affected. Until you logout.  

For example, PHP:
 ```
// $ip=$_SERVER['REMOTE_ADDR'];
$ip='1.2.3.4';
exec('/somepath/rdr_port_ipt  '.$ip.'  20  a  b');
```
For example, py3:
```
import subprocess
# import flask
# sip=flask.request.remote_addr
sip='1.2.3.4'
output=subprocess.getoutput('/somepath/rdr_port_ipt ' + sip + ' 20 a b')
```

-------
执行 `rdr_port_ipt` ：
* 根据 提供的来源 IP，在 iptables 规则表, 或nftable chain 中插入一条端口重定向规则。
* 程序驻留后台等待 N 秒(根据命令行参数)。
* 从 iptables 规则中 或nftable chain中 删除之前插入的重定向规则。
* 程序结束退出。

编译后的可执行文件，可以设置SUID (chmod 4555 ...)，由网页脚本调用，达到临时重定向端口的目的。  
如 443 -> 22 , 你可以在几秒内用 SSH 客户端连接443口，然后，即使重定向规则被删除，也不影响你的 SSH 连接。直到你主动logout。  

比如，php：
```
// $ip=$_SERVER['REMOTE_ADDR'];
$ip='1.2.3.4';
exec('/somepath/rdr_port_ipt  '.$ip.'  20  a  b');
```
如, py3:
```
import subprocess
# import flask
# sip=flask.request.remote_addr
sip='1.2.3.4'
output=subprocess.getoutput('/somepath/rdr_port_ipt ' + sip + ' 20 a b')
```

-------

2020-03-10.   
end.
