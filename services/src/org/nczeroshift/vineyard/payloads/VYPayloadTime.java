package org.nczeroshift.vineyard.payloads;

import org.nczeroshift.vineyard.VYPacket;
import org.nczeroshift.vineyard.VYPayload;

public class VYPayloadTime extends VYPayload {
    short year;
    byte month;
    byte day;
    byte hour;
    byte minute;
    byte second;

    public short getYear() {
        return year;
    }

    public void setYear(short year) {
        this.year = year;
    }

    public byte getMonth() {
        return month;
    }

    public void setMonth(byte month) {
        this.month = month;
    }

    public byte getDay() {
        return day;
    }

    public void setDay(byte day) {
        this.day = day;
    }

    public byte getHour() {
        return hour;
    }

    public void setHour(byte hour) {
        this.hour = hour;
    }

    public byte getMinute() {
        return minute;
    }

    public void setMinute(byte minute) {
        this.minute = minute;
    }

    public byte getSecond() {
        return second;
    }

    public void setSecond(byte second) {
        this.second = second;
    }

    public VYPayloadTime(){
        super();
    }

    @Override
    public void parse(String payload[]) {

    }

    @Override
    public String stream() {
        return null;
    }
}
