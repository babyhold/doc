    //1.�����ļ���   
    //import java.io.*;   
    File myFolderPath = new File(str1);   
    try {   
        if (!myFolderPath.exists()) {   
           myFolderPath.mkdir();   
        }   
    }   
    catch (Exception e) {   
        System.out.println("�½�Ŀ¼��������");   
        e.printStackTrace();   
    }    
      
    //2.�����ļ�   
    //import java.io.*;   
    File myFilePath = new File(str1);   
    try {   
        if (!myFilePath.exists()) {   
            myFilePath.createNewFile();   
        }   
        FileWriter resultFile = new FileWriter(myFilePath);   
        PrintWriter myFile = new PrintWriter(resultFile);   
        myFile.println(str2);   
        resultFile.close();   
    }   
    catch (Exception e) {   
        System.out.println("�½��ļ���������");   
        e.printStackTrace();   
    }    
      
    //3.ɾ���ļ�   
    //import java.io.*;   
    File myDelFile = new File(str1);   
    try {   
        myDelFile.delete();   
    }   
    catch (Exception e) {   
        System.out.println("ɾ���ļ���������");   
        e.printStackTrace();   
    }    
      
    //4.ɾ���ļ���   
    //import java.io.*;   
    File delFolderPath = new File(str1);   
    try {   
        delFolderPath.delete(); //ɾ�����ļ���   
    }   
    catch (Exception e) {   
        System.out.println("ɾ���ļ��в�������");   
        e.printStackTrace();   
    }    
      
    //5.ɾ��һ���ļ��¼����е��ļ���   
    //import java.io.*;   
    File delfile=new File(str1);   
    File[] files=delfile.listFiles();   
    for(int i=0;i<files.length;i++){   
        if(files[i].isDirectory()){   
            files[i].delete();   
        }   
    }     
       
    //6.����ļ���   
    //import java.io.*;   
    File delfilefolder=new File(str1);   
    try {   
        if (!delfilefolder.exists()) {   
            delfilefolder.delete();   
        }   
        delfilefolder.mkdir();   
    }   
    catch (Exception e) {   
        System.out.println("���Ŀ¼��������");   
        e.printStackTrace();   
    }    
      
    //7.��ȡ�ļ�   
    //import java.io.*;   
    // ���ж�ȡ����   
    FileReader fr = new FileReader(str1);   
    BufferedReader br = new BufferedReader(fr);   
    String str2 = br.readLine();   
    while (str2 != null) {   
        str3   
        str2 = br.readLine();   
    }   
    br.close();   
    fr.close();    
      
    //8.д���ļ�   
    //import java.io.*;   
    // ������д���ļ�   
    try {   
        FileWriter fw = new FileWriter(str1);   
        fw.write(str2);   
        fw.flush();   
        fw.close();    
    } catch (IOException e) {   
        e.printStackTrace();   
    }   
      
    //9.д������ļ�   
    //import java.io.*;   
    try {   
        RandomAcessFile logFile=new RandomAcessFile(str1,"rw");   
        long lg=logFile.length();   
        logFile.seek(str2);   
        logFile.writeByte(str3);   
    }catch(IOException ioe){   
        System.out.println("�޷�д���ļ���"+ioe.getMessage());   
    }    
      
    //10.��ȡ�ļ�����   
    //import java.io.*;   
    // �ļ����Ե�ȡ��   
    File f = new File(str1);   
    if (af.exists()) {   
        System.out.println(f.getName() + "���������£� �ļ�����Ϊ��" + f.length());   
        System.out.println(f.isFile() ? "���ļ�" : "�����ļ�");   
        System.out.println(f.isDirectory() ? "��Ŀ¼" : "����Ŀ¼");   
        System.out.println(f.canRead() ? "�ɶ�ȡ" : "��");   
        System.out.println(f.canWrite() ? "�������ļ�" : "");   
        System.out.println("�ļ��е�����޸�����Ϊ��" + new Date(f.lastModified()));   
        } else {   
        System.out.println(f.getName() + "���������£�");   
        System.out.println(f.isFile() ? "���ļ�" : "�����ļ�");   
        System.out.println(f.isDirectory() ? "��Ŀ¼" : "����Ŀ¼");   
        System.out.println(f.canRead() ? "�ɶ�ȡ" : "��");   
        System.out.println(f.canWrite() ? "�������ļ�" : "");   
        System.out.println("�ļ�������޸�����Ϊ��" + new Date(f.lastModified()));   
    }   
    if(f.canRead()){   
        str2   
    }   
    if(f.canWrite()){   
        str3   
    }   
      
    //11.д������   
    //import java.io.*;   
    File filereadonly=new File(str1);   
    try {   
        boolean b=filereadonly.setReadOnly();   
    }   
    catch (Exception e) {   
        System.out.println("�ܾ�д���ʣ�"+e.printStackTrace());   
    }    
      
    //12.ö��һ���ļ����е������ļ�   
    //import java.io.*;   
    //import java.util.*;   
    LinkedList<String> folderList = new LinkedList<String>();   
    folderList.add(str1);   
    while (folderList.size() > 0) {   
        File file = new File(folderList.peek());   
        folderList.removeLast();   
        File[] files = file.listFiles();   
        ArrayList<File> fileList = new ArrayList<File>();   
        for (int i = 0; i < files.length; i++) {   
            if (files[i].isDirectory()) {   
                folderList.add(files[i].getPath());   
            } else {   
                fileList.add(files[i]);   
            }   
        }   
        for (File f : fileList) {   
            str2=f.getAbsoluteFile();   
            str3   
        }   
    }   
      
    //13.�����ļ���    
    //import java.io.*;   
    //import java.util.*;   
    LinkedList<String> folderList = new LinkedList<String>();   
    folderList.add(str1);   
    LinkedList<String> folderList2 = new LinkedList<String>();   
    folderList2.add(str2+ str1.substring(str1.lastIndexOf("\\")));   
    while (folderList.size() > 0) {   
        (new File(folderList2.peek())).mkdirs(); // ����ļ��в����� �������ļ���   
        File folders = new File(folderList.peek());   
        String[] file = folders.list();   
        File temp = null;   
        try {   
            for (int i = 0; i < file.length; i++) {   
                if (folderList.peek().endsWith(File.separator)) {   
                    temp = new File(folderList.peek() + File.separator   
                    + file[i]);   
                } else {   
                    temp = new File(folderList.peek() + File.separator + file[i]);   
                }   
                if (temp.isFile()) {   
                    FileInputStream input = new FileInputStream(temp);   
                    FileOutputStream output = new FileOutputStream(   
                    folderList2.peek() + File.separator + (temp.getName()).toString());   
                    byte[] b = new byte[5120];   
                    int len;   
                    while ((len = input.read(b)) != -1) {   
                        output.write(b, 0, len);   
                    }   
                    output.flush();   
                    output.close();   
                    input.close();   
                }   
                if (temp.isDirectory()) {// ��������ļ���   
                    for (File f : temp.listFiles()) {   
                        if (f.isDirectory()) {   
                            folderList.add(f.getPath());   
                            folderList2.add(folderList2.peek()   
                            + File.separator + f.getName());   
                        }   
                    }   
                }   
            }   
        } catch (Exception e) {   
        //System.out.println("���������ļ������ݲ�������");   
            e.printStackTrace();   
        }   
        folderList.removeFirst();   
        folderList2.removeFirst();   
    }   
      
    //14.����һ���ļ��������е��ļ��е���һ���ļ�����   
    //import java.io.*;   
    //import java.util.*;   
    File copyfolders=new File(str1);   
    File[] copyfoldersList=copyfolders.listFiles();   
    for(int k=0;k<copyfoldersList.length;k++){   
        if(copyfoldersList[k].isDirectory()){   
            ArrayList<String>folderList=new ArrayList<String>();   
            folderList.add(copyfoldersList[k].getPath());   
            ArrayList<String>folderList2=new ArrayList<String>();   
            folderList2.add(str2+"/"+copyfoldersList[k].getName());   
            for(int j=0;j<folderList.length;j++){   
                 (new File(folderList2.get(j))).mkdirs(); //����ļ��в����� �������ļ���   
                 File folders=new File(folderList.get(j));   
                 String[] file=folders.list();   
                 File temp=null;   
                 try {   
                     for (int i = 0; i < file.length; i++) {   
                         if(folderList.get(j).endsWith(File.separator)){   
                             temp=new File(folderList.get(j)+"/"+file[i]);   
                         } else {   
                             temp=new File(folderList.get(j)+"/"+File.separator+file[i]);   
                         }   
                         FileInputStream input = new FileInputStream(temp);   
                         if(temp.isFile()){   
                             FileInputStream input = new FileInputStream(temp);   
                             FileOutputStream output = new FileOutputStream(folderList2.get(j) + "/" + (temp.getName()).toString());   
                             byte[] b = new byte[5120];   
                             int len;   
                             while ( (len = input.read(b)) != -1) {   
                                 output.write(b, 0, len);   
                             }   
                             output.flush();   
                             output.close();   
                             input.close();   
                         }   
                         if(temp.isDirectory()){//��������ļ���   
                             folderList.add(folderList.get(j)+"/"+file[i]);   
                             folderList2.add(folderList2.get(j)+"/"+file[i]);   
                         }   
                     }   
                 }   
                 catch (Exception e) {   
                     System.out.println("���������ļ������ݲ�������");   
                     e.printStackTrace();   
                 }   
            }   
        }   
    }   
      
    //15.�ƶ��ļ���   
    //import java.io.*;   
    //import java.util.*;   
    LinkedList<String> folderList = new LinkedList<String>();   
    folderList.add(str1);   
    LinkedList<String> folderList2 = new LinkedList<String>();   
    folderList2.add(str2 + str1.substring(str1.lastIndexOf("\\")));   
    while (folderList.size() > 0) {   
        (new File(folderList2.peek())).mkdirs(); // ����ļ��в����� �������ļ���   
        File folders = new File(folderList.peek());   
        String[] file = folders.list();   
        File temp = null;   
        try {   
            for (int i = 0; i < file.length; i++) {   
                if (folderList.peek().endsWith(File.separator)) {   
                    temp = new File(folderList.peek() + File.separator + file[i]);   
                } else {   
                    temp = new File(folderList.peek() + File.separator + file[i]);   
                }   
                if (temp.isFile()) {   
                    FileInputStream input = new FileInputStream(temp);   
                    FileOutputStream output = new FileOutputStream(   
                    folderList2.peek() + File.separator + (temp.getName()).toString());   
                    byte[] b = new byte[5120];   
                    int len;   
                    while ((len = input.read(b)) != -1) {   
                        output.write(b, 0, len);   
                    }   
                    output.flush();   
                    output.close();   
                    input.close();   
                    if (!temp.delete())   
                    System.out.println("ɾ�������ļ���������!");   
                }   
                if (temp.isDirectory()) {// ��������ļ���   
                    for (File f : temp.listFiles()) {   
                        if (f.isDirectory()) {   
                            folderList.add(f.getPath());   
                            folderList2.add(folderList2.peek() + File.separator + f.getName());   
                        }   
                    }   
                }   
            }   
        } catch (Exception e) {   
            // System.out.println("���������ļ������ݲ�������");   
            e.printStackTrace();   
        }   
        folderList.removeFirst();   
        folderList2.removeFirst();   
    }   
    File f = new File(str1);   
    if (!f.delete()) {   
        for (File file : f.listFiles()) {   
            if (file.list().length == 0) {   
                System.out.println(file.getPath());   
                file.delete();   
            }   
        }   
    }   
    //16.�ƶ�һ���ļ��������е��ļ��е���һ��Ŀ¼��   
    //import java.io.*;   
    //import java.util.*;   
    File movefolders=new File(str1);   
    File[] movefoldersList=movefolders.listFiles();   
    for(int k=0;k<movefoldersList.length;k++){   
        if(movefoldersList[k].isDirectory()){   
            ArrayList<String>folderList=new ArrayList<String>();   
            folderList.add(movefoldersList[k].getPath());   
            ArrayList<String>folderList2=new ArrayList<String>();   
            folderList2.add(str2+"/"+movefoldersList[k].getName());   
            for(int j=0;j<folderList.length;j++){   
                 (new File(folderList2.get(j))).mkdirs(); //����ļ��в����� �������ļ���   
                 File folders=new File(folderList.get(j));   
                 String[] file=folders.list();   
                 File temp=null;   
                 try {   
                     for (int i = 0; i < file.length; i++) {   
                         if(folderList.get(j).endsWith(File.separator)){   
                             temp=new File(folderList.get(j)+"/"+file[i]);   
                         }   
                         else{   
                             temp=new File(folderList.get(j)+"/"+File.separator+file[i]);   
                         }   
                         FileInputStream input = new FileInputStream(temp);   
                         if(temp.isFile()){   
                             FileInputStream input = new FileInputStream(temp);   
                             FileOutputStream output = new FileOutputStream(folderList2.get(j) + "/" + (temp.getName()).toString());   
                             byte[] b = new byte[5120];   
                             int len;   
                             while ( (len = input.read(b)) != -1) {   
                                 output.write(b, 0, len);   
                             }   
                             output.flush();   
                             output.close();   
                             input.close();   
                             temp.delete();   
                         }   
                         if(temp.isDirectory()){//��������ļ���   
                             folderList.add(folderList.get(j)+"/"+file[i]);   
                             folderList2.add(folderList2.get(j)+"/"+file[i]);   
                         }   
                     }   
                 }   
                 catch (Exception e) {   
                     System.out.println("���������ļ������ݲ�������");   
                     e.printStackTrace();   
                 }   
            }   
            movefoldersList[k].delete();   
        }   
    }   
      
    //17.��һ���ļ��еĿ������һ��Ŀ¼�����ļ��кͿ��ļ�   
    //import java.io.*;   
    //import java.util.*;   
    boolean b=false;//���������ļ�   
    ArrayList<String>folderList=new ArrayList<String>();   
    folderList.add(str1);   
    ArrayList<String>folderList2=new ArrayList<String>();   
    folderList2.add(str2);   
    for(int j=0;j<folderList.length;j++){   
        (new File(folderList2.get(j))).mkdirs(); //����ļ��в����� �������ļ���   
        File folders=new File(folderList.get(j));   
        String[] file=folders.list();   
        File temp=null;   
        try {   
            for (int i = 0; i < file.length; i++) {   
                if(folderList.get(j).endsWith(File.separator)){   
                    temp=new File(folderList.get(j)+"/"+file[i]);   
                }   
                else{   
                    temp=new File(folderList.get(j)+"/"+File.separator+file[i]);   
                }   
                FileInputStream input = new FileInputStream(temp);   
                if(temp.isFile()){   
                    if (b) temp.createNewFile();   
                }   
                if(temp.isDirectory()){//��������ļ���   
                    folderList.add(folderList.get(j)+"/"+file[i]);   
                    folderList2.add(folderList2.get(j)+"/"+file[i]);   
                }   
            }   
        }   
        catch (Exception e) {   
            System.out.println("���������ļ������ݲ�������");   
            e.printStackTrace();   
        }   
    }   
      
    //18.�����ļ�   
    //import java.io.*;   
     int bytesum = 0;   
     int byteread = 0;   
     File oldfile = new File(str1);   
     try {   
     if (oldfile.exists()) { //�ļ�����ʱ   
     FileInputStream inStream = new FileInputStream(oldfile); //����ԭ�ļ�   
     FileOutputStream fs = new FileOutputStream(new File(str2,oldfile.getName()));   
     byte[] buffer = new byte[5120];   
     int length;   
     while ( (byteread = inStream.read(buffer)) != -1) {   
     bytesum += byteread; //�ֽ��� �ļ���С   
     System.out.println(bytesum);   
     fs.write(buffer, 0, byteread);   
     }   
     inStream.close();   
     }   
     }   
     catch (Exception e) {   
     System.out.println("���Ƶ����ļ���������");   
     e.printStackTrace();   
     }    
      
    //19.����һ���ļ��������е��ļ�����һ��Ŀ¼   
    //import java.io.*;   
    File copyfiles=new File(str1);   
    File[] files=copyfiles.listFiles();   
    for(int i=0;i<files.length;i++){   
        if(!files[i].isDirectory()){   
            int bytesum = 0;   
            int byteread = 0;   
            try {   
                InputStream inStream = new FileInputStream(files[i]); //����ԭ�ļ�   
                FileOutputStream fs = new FileOutputStream(new File(str2,files[i].getName());   
                byte[] buffer = new byte[5120];   
                int length;   
                while ( (byteread = inStream.read(buffer)) != -1) {   
                    bytesum += byteread; //�ֽ��� �ļ���С   
                    System.out.println(bytesum);   
                    fs.write(buffer, 0, byteread);   
                }   
                inStream.close();   
            } catch (Exception e) {   
                System.out.println("���Ƶ����ļ���������");   
                e.printStackTrace();   
            }   
        }   
    }    
      
    //20.��ȡ��չ��   
    String str2=str1.substring(str1.lastIndexOf(".")+1);  