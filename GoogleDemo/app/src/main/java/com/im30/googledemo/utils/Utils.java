package com.im30.googledemo.utils;

import android.content.Intent;
import android.content.pm.ResolveInfo;
import android.util.Log;

import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Utils {
    public static ArrayList<String> patternForbiddenWords = new ArrayList<String>();

    static final char DBC_CHAR_START = 33; // 半角!
    static final char DBC_CHAR_END = 126; // 半角~
    static final char SBC_CHAR_START = 65281; // 全角！
    static final char SBC_CHAR_END = 65374; // 全角～
    static final int CONVERT_STEP = 65248; // 全角半角转换间隔
    static final char SBC_SPACE = 12288; // 全角空格 12288
    static final char DBC_SPACE = ' '; // 半角空格

    /**
     * @author dongjunjie 19/11/6
     * 敏感词汇包含类型
     * @see #containsForbiddenWords'
     */
    public static final int ForbiddenWordsType_Normal        = 1;                //正常消息，无操作
    public static final int ForbiddenWordsType_Send          = 1<<1;             //可以伪装发送聊天消息
    public static final int ForbiddenWordsType_ForceTip      = 1<<2;             //直接弹出提示，不可以伪装发送聊天消息
    public static boolean isNewShieldingEnabled = true;

    private static String pattern = "#\\d{1,2}#";

    /**
     * 是否包含违禁词汇，不论中间是否有其他字符 eg： (abcdefg) 包含 (adf)
     * 检查逻辑：遍历检查adf的每个字符在abcdefg中的index，如果d的index >
     * a的index，继续遍历，如果每个字符的index都大于上一个，则为true
     * 如果遍历中途发现index为-1（不包含这个字符）或小于上一个（顺序不对）,则返回false
     *   以{<30}#[i]#abcd(如{<30}#1#abcd)形式的检索字符串，代表必须显式发送失败，弹出提示语句
     */
    public static int containsForbiddenWords( String msg)
    {
        String checkingStr = msg.toLowerCase().trim().replaceAll(" +", " ");
        // 回车换行回避
        checkingStr = checkingStr.replace("\n","");

        for (String word : patternForbiddenWords)
        {
            // 正则表达式前加{<level},来实现等级限制
            int level = 0;
            String levelStr = "";
            try
            {
                if (word.length() > 5 && word.substring(0, 2).equals("{<"))
                {
                    if (word.substring(3, 4).equals("}"))
                    {
                        levelStr = word.substring(0, 4);
                        level = Integer.parseInt(word.substring(2, 3));
                    }
                    else if (word.substring(4, 5).equals("}"))
                    {
                        levelStr = word.substring(0, 5);
                        level = Integer.parseInt(word.substring(2, 4));
                    }
                }

                boolean needForceTip = false;
                Pattern patternReject = Pattern.compile(pattern);
                Matcher matcher = patternReject.matcher(qj2bj(word));
                if (matcher.find())
                {
                    word = matcher.replaceFirst("").trim();
                    needForceTip = true;
                }

                if (level != 0)
                {
                    if (1 < level)
                    {
                        word = word.substring(levelStr.length());
                        Pattern pattern = Pattern.compile(word);
                        if (pattern.matcher(qj2bj(checkingStr)).find())
                        {
                            Log.d("containsForbiddenWords" ,"[msg]="+ msg + " [- forbidden word:]" + word);
                            if(needForceTip){
                                return ForbiddenWordsType_ForceTip;
                            }
                            return ForbiddenWordsType_Send;
                        }
                    }
                }
                else
                {
                    Pattern pattern = Pattern.compile(word);
                    if (pattern.matcher(qj2bj(checkingStr)).find())
                    {
                        Log.d("containsForbiddenWords" ,"[msg]="+ msg + " [- forbidden word:]" + word);
                        if(needForceTip){
                            return ForbiddenWordsType_ForceTip;
                        }
                        return ForbiddenWordsType_Send;
                    }
                }
            }
            catch (Exception e)
            {

            }
        }
        return ForbiddenWordsType_Normal;
    }

    /**
     * 全角转半角
     * @param src
     * @return
     */
    public static String qj2bj(String src) {
        if (src == null) {
            return src;
        }
        StringBuilder buf = new StringBuilder(src.length());
        char[] ca = src.toCharArray();
        for (int i = 0; i < src.length(); i++) {
            if (ca[i] >= SBC_CHAR_START && ca[i] <= SBC_CHAR_END) {
                buf.append((char) (ca[i] - CONVERT_STEP));
            } else if (ca[i] == SBC_SPACE) {
                buf.append(DBC_SPACE);
            } else {
                buf.append(ca[i]);
            }
        }
        return buf.toString();
    }

    public static void addPatternForbiddenWords(String pattern)
    {
        patternForbiddenWords.add(pattern);
    }

    /**
     * 判断是否是Chrome OS 设备
     * @return true 是Chrome OS 设备
     */
    public static boolean checkIsChromeOS() {
        String services = System.getenv("SYSTEMSERVERCLASSPATH");
        if (services.contains("org.chromium.arc")) {
            return true;
        }
        return false;
    }

}
