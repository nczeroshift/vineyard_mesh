package org.nczeroshift.vineyard;

public abstract class VYPayload {
    public abstract void parse(String payload[]);
    public abstract String stream();
}
