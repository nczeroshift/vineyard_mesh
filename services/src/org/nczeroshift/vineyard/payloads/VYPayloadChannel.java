package org.nczeroshift.vineyard.payloads;

import org.nczeroshift.vineyard.VYPayload;
import org.nczeroshift.vineyard.VYUtils;

public class VYPayloadChannel extends VYPayload {
    private byte channel = 0;

    public VYPayloadChannel(byte channel) {
        this.channel = channel;
    }

    @Override
    public void parse(String[] payload) {

    }

    @Override
    public String stream() {
        return VYUtils.bytesToHex(new byte[]{channel});
    }

    public byte getChannel() {
        return channel;
    }

    public void setChannel(byte channel) {
        this.channel = channel;
    }
}
