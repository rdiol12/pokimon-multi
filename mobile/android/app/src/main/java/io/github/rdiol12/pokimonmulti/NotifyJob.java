package io.github.rdiol12.pokimonmulti;

import android.app.job.JobParameters;
import android.app.job.JobService;

/** Checks the server for new notes now and then while the app isn't running (see {@link Notifier}). */
public class NotifyJob extends JobService {
    @Override
    public boolean onStartJob(JobParameters params) {
        Thread t = new Thread(() -> {
            Notifier.poll(getApplicationContext());
            jobFinished(params, false);
        }, "notify-poll");
        t.start();
        return true;
    }

    @Override
    public boolean onStopJob(JobParameters params) {
        return true;
    }
}
