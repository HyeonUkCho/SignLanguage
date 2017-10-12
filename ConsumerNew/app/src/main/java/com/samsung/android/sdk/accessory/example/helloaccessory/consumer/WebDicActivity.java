package com.samsung.android.sdk.accessory.example.helloaccessory.consumer;


import android.app.Activity;
import android.os.Bundle;
import android.webkit.WebView;
import android.webkit.WebViewClient;

public class WebDicActivity extends Activity {
    private WebView mWebView;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_web_dic);

        setLayout();
        mWebView.getSettings().setJavaScriptEnabled(true); //웹뷰에서 자바스크립트 실행가능
        mWebView.loadUrl("http://sldict.korean.go.kr/front/main/main.do"); //구글홈페이지 지정
        mWebView.setWebViewClient(new WebViewClientClass()); //webviewclient 지정
    }
    private void setLayout() {
        mWebView = (WebView) findViewById(R.id.webview);
    }


    class WebClient extends WebViewClient {
        public boolean shouldOverrideUrlLoading(WebView view, String url) {
            view.loadUrl(url);
            return true;
        }

    }

    private class WebViewClientClass extends WebViewClient {
    }
}

