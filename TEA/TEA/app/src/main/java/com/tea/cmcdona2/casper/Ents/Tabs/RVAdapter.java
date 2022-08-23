package com.tea.cmcdona2.casper.Ents.Tabs;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.support.v7.widget.CardView;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.tea.cmcdona2.casper.Ents.EntsActivity;
import com.tea.cmcdona2.casper.Ents.Tabs.Ent_CardItem;
import com.tea.cmcdona2.casper.ParticularEnt.ParticularEntActivity;
import com.tea.cmcdona2.casper.R;

import java.util.List;

public class RVAdapter extends RecyclerView.Adapter<RVAdapter.PersonViewHolder>{

    static int position;
    static int numOfEvents;
    static List<Ent_CardItem> EntCards;
   static String filteredIDs1;
    static String eventPositions1;

    public static class PersonViewHolder extends RecyclerView.ViewHolder {
        CardView cv;
        TextView personName;
        TextView personAge;
        ImageView personPhoto;


        PersonViewHolder(View itemView) {
            super(itemView);
            cv = (CardView)itemView.findViewById(R.id.cv);
            personName = (TextView)itemView.findViewById(R.id.event_title);
            personAge = (TextView)itemView.findViewById(R.id.event_timming);
            personPhoto = (ImageView)itemView.findViewById(R.id.event_poster);

            itemView.setOnClickListener(new View.OnClickListener() {
                @Override public void onClick(View v) {
                    final SharedPreferences appPrefs = v.getContext().getSharedPreferences("appPrefs", 0);
                    String loadedID = filteredIDs1;


                    String Event = personName.getText().toString();

                    final String swipeEventId = loadedID;
                    Intent intent = new Intent(v.getContext(), ParticularEntActivity.class);
                    intent.putExtra("Event", Event);
                    intent.putExtra("swipeEventId", swipeEventId);
                    intent.putExtra("swipeCount", EntCards.size());
                    position = getAdapterPosition();
                    intent.putExtra("swipePosition", position);
                    intent.putExtra("eventPosition", eventPositions1 );
                    v.getContext().startActivity(intent);
                }
            });
        }
    }



    RVAdapter(List<Ent_CardItem> EntCards, String filteredIDs, String eventPositions){
        this.EntCards = EntCards;
        filteredIDs1 = filteredIDs;
        eventPositions1 = eventPositions;

    }

    @Override
    public int getItemCount() {
        return EntCards.size();
    }

    @Override
    public PersonViewHolder onCreateViewHolder(ViewGroup viewGroup, int i) {
        View v = LayoutInflater.from(viewGroup.getContext()).inflate(R.layout.card_ent_item, viewGroup, false);
        PersonViewHolder pvh = new PersonViewHolder(v);
        return pvh;
    }


    @Override
    public void onBindViewHolder(PersonViewHolder personViewHolder, int i) {
        personViewHolder.personName.setText(EntCards.get(i).event_name);
        personViewHolder.personAge.setText(EntCards.get(i).event_timing);
        personViewHolder.personPhoto.setImageBitmap(EntCards.get(i).event_image);
    }

    @Override
    public void onAttachedToRecyclerView(RecyclerView recyclerView) {
        super.onAttachedToRecyclerView(recyclerView);
    }
}