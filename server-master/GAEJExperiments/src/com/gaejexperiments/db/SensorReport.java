package com.gaejexperiments.db;

import java.util.Date;
import com.google.appengine.api.datastore.Key;

import javax.jdo.annotations.IdGeneratorStrategy;
import javax.jdo.annotations.IdentityType;
import javax.jdo.annotations.PersistenceCapable;
import javax.jdo.annotations.Persistent;
import javax.jdo.annotations.PrimaryKey;

@PersistenceCapable(identityType = IdentityType.APPLICATION)
public class SensorReport {
	@PrimaryKey
	@Persistent(valueStrategy = IdGeneratorStrategy.IDENTITY)
	private Key key;
	
	private String nodeID;
	@Persistent
	private String tagCode;
	@Persistent
	private String sensor1Value;
	@Persistent
	private String sensor2Value;
	@Persistent
	private String sensor3Value;
	@Persistent
	private Date reportDateTime;

	public SensorReport(String nodeID,String tagCode, String sensor1Value, String sensor2Value, String sensor3Value, Date reportDateTime) {
		super();
		this.tagCode = tagCode;
		this.nodeID=nodeID;
		this.sensor1Value = sensor1Value;
		this.sensor2Value= sensor2Value;
		this.sensor3Value = sensor3Value;
		this.reportDateTime = reportDateTime;
	}
	
	public Key getKey() {
		return key;
	}

	public void setKey(Key key) {
		this.key = key;
	}

	public String getTagCode() {
		return tagCode;
	}

	public void setTagCode(String tagCode) {
		this.tagCode = tagCode;
	}
	
	public String getNodeID() {
		return tagCode;
	}

	public void setNodeID(String nodeID) {
		this.nodeID = nodeID;
	}
	
	public String getSensor1Value() {
		return sensor1Value;
	}

	public void setSensor1Value(String sensor1Value) {
		this.sensor1Value = sensor1Value;
	}
	public String getSensor2Value() {
		return sensor2Value;
	}

	public void setSensor2Value(String sensor2Value) {
		this.sensor2Value = sensor2Value;
	}
	public String getSensor3Value() {
		return sensor3Value;
	}

	public void setSensorValue(String sensor3Value) {
		this.sensor3Value = sensor3Value;
	}

	public Date getReportDateTime() {
		return reportDateTime;
	}

	public void setReportDateTime(Date reportDateTime) {
		this.reportDateTime = reportDateTime;
	}
}