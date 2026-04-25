package com.aiplayer.mobile

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import aiplayer.ui.mobile.android.app.AIPlayerAndroidApp

class AndroidMainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            AIPlayerAndroidApp()
        }
    }
}
