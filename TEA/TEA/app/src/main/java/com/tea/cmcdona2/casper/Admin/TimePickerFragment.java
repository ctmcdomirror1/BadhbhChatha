package com.tea.cmcdona2.casper.Admin;

        import android.app.Dialog;
        import android.app.DialogFragment;
        import android.app.TimePickerDialog;
        import android.os.Bundle;
        import android.text.format.DateFormat;
        import android.widget.TimePicker;

        import java.text.SimpleDateFormat;
        import java.util.Calendar;
        import java.util.Locale;

/**
 * Created by Abhinav on 12-Mar-16.
 */
public class TimePickerFragment extends DialogFragment
        implements TimePickerDialog.OnTimeSetListener {

    TheTimeListener listener;

    public interface TheTimeListener{
        public void returnTime(String date);
    }
    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        // Use the current time as the default values for the picker
        final Calendar c = Calendar.getInstance();
        int hour = c.get(Calendar.HOUR_OF_DAY);
        int minute = c.get(Calendar.MINUTE);
        listener = (TheTimeListener) getActivity();
        // Create a new instance of TimePickerDialog and return it
        return new TimePickerDialog(getActivity(), this, hour, minute,
                DateFormat.is24HourFormat(getActivity()));
    }

    public void onTimeSet(TimePicker view, int hourOfDay, int minute) {
        // Do something with the time chosen by the user

        Calendar c = Calendar.getInstance();
        c.set(0,0,0,hourOfDay,minute);

        SimpleDateFormat sdf = new SimpleDateFormat("hh:mm a", Locale.UK);
        String formattedDate = sdf.format(c.getTime());
        if (listener != null)
        {
            listener.returnTime(formattedDate);

        }
    }
}