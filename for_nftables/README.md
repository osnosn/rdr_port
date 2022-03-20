This is for nftable.

--------
Compile with `gcc -O -o rdr_port_nft  rdr_port_nft.c`   

This program is suitable for CentOS-8, firewalld enabled. Because firewalld has built many NFT tables and chains.  
Otherwise, it needs to be modified.  

Execute `rdr_port_nft` :
* Insert a port redirection rule in the nftable chain according to the source IP provided. And get the handle NUM.
  * Like this: `nft -ea add rule ip nat PREROUTING ip saddr 1.2.3.4 tcp dport 443 redirect to 22`
* The program resides in the background and waits for N seconds (depending on the command line parameters).
* Remove the previously inserted redirection rule from the nftable chain.
  * Like this: `nft delete rule ip nat PREROUTING handle 10`
* The program done & exit.

-------
用 `gcc -O -o rdr_port_ipt  rdr_port_ipt.c` 编译   

此程序，适合 CentOS-8，firewalld enabled 的情况。因为firewalld 已经建好了许多 nft 的 tables 和 chains。  
否则需要修改才能用。  

执行 `rdr_port_ipt` ：
* 根据 提供的来源 IP，在 nftable chain 中插入一条端口重定向规则。并获取 handle 值。
  * 例如: `nft -ea add rule ip nat PREROUTING ip saddr 1.2.3.4 tcp dport 443 redirect to 22`
* 程序驻留后台等待 N 秒(根据命令行参数)。
* 从 nftable chain 中删除之前插入的重定向规则。
  * 例如: `nft delete rule ip nat PREROUTING handle 10`
* 程序结束退出。

-------
In CentOS-8 , firewalld.service enabled.  
The system has created many NFT tables and chains.  
系统已经创建了许多 NFT 的表和链。  
```
table .... {
  ....
}
....
table ip nat {
   chain PREROUTING {
      type nat hook prerouting priority -100; policy accept;
   }
   chain INPUT {
      type nat hook input priority 100; policy accept;
   }
   chain POSTROUTING {
      type nat hook postrouting priority 100; policy accept;
   }
   chain OUTPUT {
      type nat hook output priority -100; policy accept;
   }
}
....
table .... {
  ....
}
```
This program add a rule into `table ip nat` -> `chain PREROUTING` .  
So, if you don't have these tables and chains in your system, or they are different.  
You need to create the appropriate tables and chains yourself, and modify the program to insert the rule into the correct chain.  
Because `table ip nat` only supports IPv4, the program only supports IPv4.  
If you need IPv6, modify program, add the rule into `table ip6 nat` ->  `chain PREROUTING` .  

此程序把规则加入到 `ip nat` 表，`PREROUTING` 链中。  
如果你的系统中没有这些表和链，或者不同于这些表和链。  
你需要自己建立合适的 表和链，并修改程序，把规则插入正确的链中。  
因为 `table ip nat` 只支持IPv4, 所以程序只支持IPv4.   
如要支持IPv6，请修改程序，把rule加入 `table ip6 nat` -> `chain PREROUTING` 中。  

-------
In Debian-11 ,   
Install nftables, `apt install nftables`,   
Enable nftables service, `systemctl enable nftables`   
MODIFY `/etc/nftables.conf`, ADD chain and hook into nftables `type nat hook prerouting priority 10;`   
```
table inet filter {
	chain input {
		type filter hook input priority 10;
	}
	chain forward {
		type filter hook forward priority 10;
	}
	chain output {
		type filter hook output priority 10;
	}
	chain prerouting {
		type nat hook prerouting priority 10;
	}
}
```
RELOAD nftables, `nft -f /etc/nftables.conf`   

File `rdr_port_nft.c` line 151,   
* `execl("/usr/sbin/nft","nft","-eann","add","rule","ip","nat","PREROUTING","ip","saddr",ip,"tcp","dport",sport,"redirect","to",dport,(char *) NULL);`   

MODIFY to (修改为)   
* `execl("/usr/sbin/nft","nft","-eann","add","rule","inet","filter","prerouting","ip","saddr",ip,"tcp","dport",sport,"redirect","to",dport,(char *) NULL);`   

Compile with `gcc -O -o rdr_port_nft  rdr_port_nft.c`   


-------
2022-03-20.   
end.  
