#gStore����Review��¼

##gStore0.7.1.dev(2018-09-30)

###�ܽ�

1. Ŀǰ�����ѫ���ŵ����������ɣ��߼���ȷ�������ύ��dev�汾��


2. ���ڵ���Ҫ����
	1. ����ע�ͽ��٣���
	
			else if(type == "change_psw")
			{
			pthread_rwlock_wrlock(&users_map_lock);
			std::map<std::string, struct User *>::iterator iter;
			iter = users.find(username2);
		
			if(iter == users.end())
			{
				string error = "username not exist, change password failed.";
				string resJson = CreateJson(916, error, 0);
				*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json" << "\r\n\r\n" << resJson;
				pthread_rwlock_unlock(&users_map_lock);
		
				return false;
				
			}
			else
			{
				iter->second->setPassword(password2);
			}
			pthread_rwlock_unlock(&users_map_lock);

			}

	��˴���Ӧ�ý��ϱ�Ҫ�Ĵ���

 	

  2. ȱ�ٱ�Ҫ��try catch�쳣���񼰴�����룬��
			
			else if(type == "change_psw")
			{
			pthread_rwlock_wrlock(&users_map_lock);
			std::map<std::string, struct User *>::iterator iter;
			iter = users.find(username2);
		
			if(iter == users.end())
			{
				string error = "username not exist, change password failed.";
				string resJson = CreateJson(916, error, 0);
				*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json" << "\r\n\r\n" << resJson;
				pthread_rwlock_unlock(&users_map_lock);
		
				return false;
				
			}
			else
			{
				iter->second->setPassword(password2);
			}
			pthread_rwlock_unlock(&users_map_lock);

			}
  �˴�����������쳣�Ļ�����ô`pthread_rwlock_unlock`�Ƿ����ִ�У��������ִ�еĻ����Ƿ�ᵼ�������ܽ�����

###��Ĵ���Rewiew���

1. �������ά��
>����û�����⣬�����¿��ԸĽ��ĵط�
>>1. ���в�����`success`��Ӧ����һ��code����Ϊ�û�ֻ��Ҫ��ע�Ƿ�ɹ����ɹ��˵Ļ�������`msg`�����ɿͻ��Լ�ȥ��֯,��ȻҲ���������ṩ���ٸ�����

		json.put("errorcode",0);
        json.put("errormsg","load database successful")

>>2 ���Խ�������������md�ļ�������Ŀ�ļ����棬����ʹ���߲�ѯ

2.JSON��װ
>Ŀǰ���õ���boost��json��ط�������Ȼ�ǿ��ԣ���������Ҫ���ǵ��ǣ�ĿǰgStore��boost������̫ǿ�����˸о�������boost����һ���������Ŀ⣨��װ�ļ���1G���ϣ��������Ļ���ϵͳ��϶�̫�߲����ڽ�������չ�����Ƽ�������Ѷ�Ŀ�Դ[RapidJSON](http://rapidjson.org "RapidJSON����"),���Է���[GitHub](https://github.com/Tencent/rapidjson/ "GitHub")��Ŀǰ�ų���������ߵ�c++JSON�⣬������Java��JSON��[fastJSON](https://github.com/alibaba/fastjson "fastJSON github")

3.���밲ȫ
>Ŀǰ�����޸ĵ�user��password����`map`�ķ�ʽ�洢���ڴ��У���ghttp����֮��password�ֻỹԭ��������Ҫ���ģ�����ʦ����������ݿ��д���һ��`system.db`���ݿ⣬���ڱ���ϵͳ��Ϣ�Ͳ������ݣ��˷�������


###��ѫ�����Review���
1. ghttp������������
>������Ĭ��ֵ�������߼�����������

2.ghttp�ر�
>������`shutdown`ָ�����ghttp��servlet�����ṩ��stop�ӿڹ��ⲿ�ӿڵ��ã�ʹ�����ⲿ�ӿڵ�Զ�̹ر�ghttp����

3.ghttp���ݿ�״̬ά��
>�������ع���`stop`��`shop`��`drop`��`checkpoint`��`checkall`�Ƚӿڣ������������󣬵���Ҫ������ؽӿ�������������[API.md](..\API.md)�����������




